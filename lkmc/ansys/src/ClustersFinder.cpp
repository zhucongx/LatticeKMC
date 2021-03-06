#include "ClustersFinder.h"

#include <queue>
#include <unordered_map>
#include <utility>
#include <filesystem>
#include <omp.h>
namespace ansys {
ClustersFinder::ClustersFinder(const cfg::Config &config,
                               Element solvent_atom_type,
                               size_t smallest_cluster_criteria,
                               // size_t solvent_bond_criteria,
                               const pred::TotalEnergyPredictor &energy_estimator)
    : config_(config),
      solvent_config_(config),
      solvent_element_(solvent_atom_type),
      element_set_(config_.GetElementSetWithoutVacancy()),
      smallest_cluster_criteria_(smallest_cluster_criteria),
      // solvent_bond_criteria_(solvent_bond_criteria),
      energy_estimator_(energy_estimator) {
  for (size_t atom_id = 0; atom_id < solvent_config_.GetNumAtoms(); ++atom_id) {
    solvent_config_.ChangeAtomElementTypeAtAtom(atom_id, Element("Al"));
  }
  absolute_energy_solvent_config_ = energy_estimator_.GetEnergy(solvent_config_);
}

ClustersFinder::ClusterElementNumMap ClustersFinder::FindClustersAndOutput(
    const std::string &output_folder, const std::string &output_name) {
  auto cluster_to_atom_vector = FindAtomListOfClusters();

  ClusterElementNumMap cluster_element_num_map;
  std::vector<cfg::Lattice> lattice_vector;
  std::vector<cfg::Atom> atom_vector;
#pragma omp parallel  default(none) shared(cluster_to_atom_vector, atom_vector, lattice_vector, cluster_element_num_map)
  {
#pragma omp for
    for (auto &atom_list: cluster_to_atom_vector) {
      const double cluster_energy = GetRelativeEnergyOfCluster(atom_list);
      // initialize map with all the element, because some cluster may not have all types of element
      std::map<Element, size_t> num_atom_in_one_cluster;
      for (const auto &element: element_set_) {
        num_atom_in_one_cluster[element] = 0;
      }
#pragma omp critical
      {
        for (const auto &atom_index: atom_list) {
          Element type = config_.GetAtomVector()[atom_index].GetElement();
          num_atom_in_one_cluster[type]++;
          atom_vector.emplace_back(atom_vector.size(), type);
          auto relative_position =
              config_.GetLatticeVector()[config_.GetLatticeIdFromAtomId(atom_index)].GetRelativePosition();
          lattice_vector.emplace_back(lattice_vector.size(),
                                      relative_position * config_.GetBasis(), relative_position);
        }
        cluster_element_num_map.emplace_back(
            std::make_pair(num_atom_in_one_cluster, cluster_energy));
      }
    }
  }
  cfg::Config config_out(config_.GetBasis(), lattice_vector, atom_vector, false);

  if (output_folder.empty()) {
    config_out.WriteCfg(output_name, false);
  } else {
    std::filesystem::create_directories(output_folder);
    config_out.WriteCfg(output_folder + "/" + output_name, false);
  }
  return cluster_element_num_map;
}

std::unordered_set<size_t> ClustersFinder::FindSoluteAtomIndexes() const {
  std::unordered_set<size_t> solute_atoms_hashset;
  for (const auto &atom: config_.GetAtomVector()) {
    if (atom.GetElement() == solvent_element_ || atom.GetElement() == ElementName::X) { continue; }
    solute_atoms_hashset.insert(atom.GetId());
  }
  return solute_atoms_hashset;
}

std::vector<std::vector<size_t> > ClustersFinder::FindAtomListOfClustersBFSHelper(
    std::unordered_set<size_t> unvisited_atoms_id_set) const {
  std::vector<std::vector<size_t> > cluster_atom_list;
  std::queue<size_t> visit_id_queue;
  size_t atom_id;

  std::unordered_set<size_t>::iterator it;
  while (!unvisited_atoms_id_set.empty()) {
    // Find next element
    it = unvisited_atoms_id_set.begin();
    visit_id_queue.push(*it);
    unvisited_atoms_id_set.erase(it);

    std::vector<size_t> atom_list_of_one_cluster;
    while (!visit_id_queue.empty()) {
      atom_id = visit_id_queue.front();
      visit_id_queue.pop();

      atom_list_of_one_cluster.push_back(atom_id);
      for (const auto &neighbors_list: {
          config_.GetFirstNeighborsAtomIdVectorOfAtom(atom_id),
          // config_.GetSecondNeighborsAtomIdVectorOfAtom(atom_id)
      }) {
        for (auto neighbor_id: neighbors_list) {
          it = unvisited_atoms_id_set.find(neighbor_id);
          if (it != unvisited_atoms_id_set.end()) {
            visit_id_queue.push(*it);
            unvisited_atoms_id_set.erase(it);
          }
        }
      }
    }
    cluster_atom_list.push_back(atom_list_of_one_cluster);
  }
  return cluster_atom_list;
}

std::vector<std::vector<size_t> > ClustersFinder::FindAtomListOfClusters() const {
  auto cluster_atom_list = FindAtomListOfClustersBFSHelper(FindSoluteAtomIndexes());

  // remove small clusters
  auto it = cluster_atom_list.begin();
  while (it != cluster_atom_list.end()) {
    if (it->size() <= smallest_cluster_criteria_) {
      it = cluster_atom_list.erase(it);
    } else {
      ++it;
    }
  }

  /*
  std::unordered_set<size_t> all_found_solute_set;
  for (const auto &singe_cluster_vector: cluster_atom_list) {
    std::copy(singe_cluster_vector.begin(),
              singe_cluster_vector.end(),
              std::inserter(all_found_solute_set, all_found_solute_set.end()));
  }

  // add solvent neighbors
  for (const auto &atom: config_.GetAtomVector()) {
    if (atom.GetElement() != solvent_element_)
      continue;
    size_t neighbor_bond_count = 0;
    for (auto neighbor_id: config_.GetFirstNeighborsAtomIdVectorOfAtom(atom.GetId())) {
      const auto &neighbor_type = config_.GetAtomVector()[neighbor_id].GetElement();
      if (neighbor_type != solvent_element_ && neighbor_type != ElementName::X
          && all_found_solute_set.find(neighbor_id) != all_found_solute_set.end())
        neighbor_bond_count++;
    }
    if (neighbor_bond_count >= solvent_bond_criteria_)
      all_found_solute_set.insert(atom.GetId());
  }

  cluster_atom_list = FindAtomListOfClustersBFSHelper(all_found_solute_set);
  */
  return cluster_atom_list;
}
double ClustersFinder::GetAbsoluteEnergyOfCluster(const std::vector<size_t> &atom_id_list) const {
  cfg::Config solute_config(solvent_config_);
  for (size_t atom_id: atom_id_list) {
    solute_config.ChangeAtomElementTypeAtAtom(atom_id, config_.GetElementAtAtomId(atom_id));
  }
  // solute_config.WriteCfg("cluster/" + std::to_string(atom_id_list.size()) + ".cfg", false);
  return energy_estimator_.GetEnergy(solute_config);
}
double ClustersFinder::GetRelativeEnergyOfCluster(const std::vector<size_t> &atom_id_list) const {
  cfg::Config solute_config(solvent_config_);
  for (size_t atom_id: atom_id_list) {
    solute_config.ChangeAtomElementTypeAtAtom(atom_id, config_.GetElementAtAtomId(atom_id));
  }
  return energy_estimator_.GetEnergyOfCluster(solute_config, atom_id_list) -
      energy_estimator_.GetEnergyOfCluster(solvent_config_, atom_id_list)
      + absolute_energy_solvent_config_;
}
} // namespace kn
