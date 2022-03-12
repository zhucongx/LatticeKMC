#include "EnergyPredictorLru.h"

namespace pred {
pred::EnergyPredictorLru::EnergyPredictorLru(const std::string &predictor_filename,
                                             const cfg::Config &reference_config,
                                             const std::set<Element> &type_set,
                                             size_t cache_size)
    : EnergyPredictor(predictor_filename, reference_config, type_set), cache_size_(cache_size) {}
EnergyPredictorLru::~EnergyPredictorLru() = default;
std::pair<double, double> EnergyPredictorLru::GetBarrierAndDiffFromLatticeIdPair(
    const cfg::Config &config,
    const std::pair<size_t, size_t> &lattice_id_jump_pair) const {
  auto hash_value = GetHashFromConfigAndLatticeIdPair(config, lattice_id_jump_pair);
  std::pair<double, double> value;
  auto it = hashmap_.find(hash_value);
  if (it == hashmap_.end()) {
    value = EnergyPredictor::GetBarrierAndDiffFromLatticeIdPair(config, lattice_id_jump_pair);
    Add(hash_value, value);
  } else {
    cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
    value = it->second->second;
  }
  return value;
}
size_t EnergyPredictorLru::GetHashFromConfigAndLatticeIdPair(
    const cfg::Config &config,
    const std::pair<size_t, size_t> &lattice_id_jump_pair) const {
  const auto &cluster_mapping = site_bond_mapping_hashmap_.at(lattice_id_jump_pair);
  size_t seed = 0;
  for (size_t i = 0; i < constants::kNumThirdNearestSetSize; i++) {
    boost::hash_combine(seed, config.GetAtomIdFromLatticeId(cluster_mapping[0][i][0]));
  }
  return seed;
}
void EnergyPredictorLru::Add(size_t key, std::pair<double, double> value) const {
  auto it = hashmap_.find(key);
  if (it != hashmap_.end()) {
    cache_list_.erase(it->second);
  }
  cache_list_.push_front(std::make_pair(key, std::move(value)));
  hashmap_[key] = cache_list_.begin();
  if (hashmap_.size() > cache_size_) {
    auto last = cache_list_.rbegin()->first;
    cache_list_.pop_back();
    hashmap_.erase(last);
  }
}
} // namespace pred