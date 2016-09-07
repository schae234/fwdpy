#include <algorithm>
#include <stdexcept>

#include "allele_ages.hpp"
#include <algorithm>
#include <iostream>
using namespace std;

namespace fwdpy
{
    vector<allele_age_data_t>
    allele_ages_details(const selected_mut_tracker::final_t &trajectories,
                        const double minfreq, const unsigned minsojourn)
    {
        if (minfreq < 0.0)
            throw runtime_error("minfreq must be >= 0.0");
        vector<allele_age_data_t> rv;
        using element_t = std::pair<unsigned, double>;
        for (const auto &t : *trajectories)
            {
                // decltype(t.first) is selected_mut_data
                // decltype(t.second) is vector<double>, and is the vec of
                // recorded frequencies
                if (t.second.empty())
                    {
                        throw runtime_error("frequency vector empty");
                    }
                if (t.second.size() >= minsojourn)
                    {
                        auto mfi = max_element(
                            t.second.begin(), t.second.end(),
                            [](const element_t &a, const element_t &b) {
                                return a.second <= b.second;
                            });
                        if (mfi->second
                            >= minfreq) // it hit the right minimum frequency
                            {
                                rv.emplace_back(t.first.esize, mfi->second,
                                                t.second.back().second,
                                                t.first.origin,
                                                t.second.size());
                            }
                    }
            }
        return rv;
    }

    selected_mut_tracker::final_t
    merge_trajectories_details(const selected_mut_tracker::final_t &traj1,
                               const selected_mut_tracker::final_t &traj2)
    {
        selected_mut_tracker::final_t rv(
            new selected_mut_tracker::final_t::element_type(traj1->begin(),
                                                            traj1->end()));
        for (auto &&t : *traj2)
            {
                auto x = std::find_if(
                    rv->begin(), rv->end(),
                    [&t](const selected_mut_tracker::final_t::element_type::
                             value_type &xi) { return xi.first == t.first; });
                if (x == rv->end())
                    {
                        rv->push_back(t);
                    }
                else
                    {
                        x->second.insert(x->second.end(), t.second.begin(),
                                         t.second.end());
                    }
            }
        return rv;
    }

    std::vector<selected_mut_data_tidy>
    tidy_trajectory_info(const selected_mut_tracker::final_t &trajectories,
                         const unsigned min_sojourn, const double min_freq,
                         // Warning: these next two are only accurate
                         // to the extent that frequencies were sampled
                         // each generation!
                         const unsigned remove_gone_before,
                         const unsigned remove_arose_after)
    {
        std::vector<selected_mut_data_tidy> rv;
        for (const auto &ti : *trajectories)
            {
                // Make sure that sojourn time filter is not applied to
                // fixations, as
                // those are usually of particular interest.
                if (!ti.second.empty() && (ti.second.size() >= min_sojourn
                                           || ti.second.back().second == 1.0))
                    {
                        if (ti.first.origin <= remove_arose_after)
                            {
                                if (ti.second.back().first
                                    > remove_gone_before)
                                    {
                                        using element_t
                                            = std::pair<unsigned, double>;
                                        auto mx = max_element(
                                            ti.second.begin(), ti.second.end(),
                                            [](const element_t &a,
                                               const element_t &b) {
                                                return a.second <= b.second;
                                            });
                                        if (mx->second >= min_freq)
                                            {
                                                for (const auto &f : ti.second)
                                                    {
                                                        rv.emplace_back(
                                                            ti.first.origin,
                                                            f.first,
                                                            ti.first.pos,
                                                            f.second,
                                                            ti.first.esize,
                                                            ti.first.label);
                                                    }
                                            }
                                    }
                            }
                    }
            }
        return rv;
    }
}
