#ifndef LKMC_LKMC_PRED_INCLUDE_ENERGYPREDICTORE0DECLUSTER_H_
#define LKMC_LKMC_PRED_INCLUDE_ENERGYPREDICTORE0DECLUSTER_H_
#include "EnergyPredictor.h"
#include "ElementCluster.hpp"
namespace pred {

class EnergyPredictorE0DECluster : public EnergyPredictor {
  public:
    EnergyPredictorE0DECluster(const std::string &predictor_filename,
                               const cfg::Config &reference_config,
                               const std::set<Element> &type_set);
    ~EnergyPredictorE0DECluster() override;
  protected:
    [[nodiscard]] std::pair<double, double> GetBarrierAndDiffFromLatticeIdPair(
        const cfg::Config &config,
        const std::pair<size_t, size_t> &lattice_id_jump_pair) const override;
    [[nodiscard]] double GetE0(const cfg::Config &config,
                               const std::pair<size_t, size_t> &lattice_id_jump_pair,
                               Element migration_element) const;
    [[nodiscard]] double GetDE(const cfg::Config &config,
                               const std::pair<size_t, size_t> &lattice_id_jump_pair,
                               Element migration_element) const;
  private:
    const std::vector<std::vector<std::vector<size_t> > > mapping_mmm_{};
    const std::vector<std::vector<std::vector<size_t> > > mapping_periodic_{};

    std::unordered_map<std::pair<size_t, size_t>,
                       std::vector<size_t>,
                       boost::hash<std::pair<size_t, size_t> > > site_bond_cluster_mmm_hashmap_;
    std::unordered_map<size_t,
                       std::vector<size_t> > site_cluster_periodic_hashmap_;

    std::unordered_map<Element,
                       ParametersE0,
                       boost::hash<Element>> element_parameters_hashmap_;
    std::vector<double> theta_cluster_{};

    const std::unordered_map<ElementCluster, size_t,
                             boost::hash<ElementCluster> > initialized_cluster_hashmap_;

};

} // namespace pred
#endif //LKMC_LKMC_PRED_INCLUDE_ENERGYPREDICTORE0DECLUSTER_H_
