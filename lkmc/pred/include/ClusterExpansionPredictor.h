#ifndef LKMC_LKMC_PRED_INCLUDE_CLUSTEREXPANSIONPREDICTOR_H_
#define LKMC_LKMC_PRED_INCLUDE_CLUSTEREXPANSIONPREDICTOR_H_
#include <string>
#include <set>
#include <boost/functional/hash.hpp>
#include "Config.h"
#include "LatticeCluster.hpp"
#include "ElementCluster.hpp"
namespace pred {

std::vector<std::vector<std::vector<size_t> > > GetClusterParametersMappingState(
    const cfg::Config &config, const std::pair<size_t, size_t> &lattice_id_jump_pair);
std::vector<cfg::Lattice> GetSortedLatticeVectorState(
    const cfg::Config &config, const std::pair<size_t, size_t> &lattice_id_jump_pair);
std::array<std::vector<double>, 2> GetEncodesFromMapState(
    const cfg::Config &config,
    const std::pair<size_t, size_t> &lattice_id_jump_pair,
    const std::unordered_map<ElementCluster,
                             int,
                             boost::hash<ElementCluster> > &initialized_cluster_hashmap,
    const std::vector<std::vector<std::vector<size_t> > > &cluster_mapping);

} // namespace pred

#endif //LKMC_LKMC_PRED_INCLUDE_CLUSTEREXPANSIONPREDICTOR_H_
