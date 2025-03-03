/*
 * CryptoMiniSat
 *
 * Copyright (c) 2009-2013, Mate Soos. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
*/

#ifndef SOLVER_H
#define SOLVER_H

#include "constants.h"
#include <vector>

#include "constants.h"
#include "solvertypes.h"
#include "implcache.h"
#include "propengine.h"
#include "searcher.h"
#include "GitSHA1.h"
#include <fstream>

namespace CMSat {

using std::vector;
using std::pair;
using std::string;

class VarReplacer;
class ClauseCleaner;
class Prober;
class Simplifier;
class SCCFinder;
class Vivifier;
class Strengthener;
class CalcDefPolars;
class SolutionExtender;
class SQLStats;
class ImplCache;
class CompFinder;
class CompHandler;
class SubsumeStrengthen;
class SubsumeImplicit;
class DataSync;
class SharedData;

class LitReachData {
    public:
        LitReachData() :
            lit(lit_Undef)
            , numInCache(0)
        {}
        Lit lit;
        uint32_t numInCache;
};


class Solver : public Searcher
{
    public:
        Solver(const SolverConf _conf = SolverConf(), bool* _needToInterrupt = NULL);
        ~Solver() override;

        void add_sql_tag(const string& tagname, const string& tag);
        const vector<std::pair<string, string> >& get_sql_tags() const;
        void new_external_var();
        void new_external_vars(size_t n);
        bool add_clause_outer(const vector<Lit>& lits);
        bool add_xor_clause_outer(const vector<Var>& vars, bool rhs);

        lbool solve_with_assumptions(const vector<Lit>* _assumptions = NULL);
        void  set_shared_data(SharedData* shared_data, uint32_t thread_num);
        lbool modelValue (const Lit p) const;  ///<Found model value for lit
        const vector<lbool>& get_model() const;
        const vector<Lit>& get_final_conflict() const;

        struct SolveStats
        {
            uint64_t numSimplify = 0;
            uint64_t nbReduceDB = 0;
            uint32_t num_solve_calls = 0;
        };
        static const char* getVersion();
        vector<Lit> get_zero_assigned_lits() const;
        void     open_file_and_dump_irred_clauses(string fname) const;
        void     open_file_and_dump_red_clauses(string fname) const;
        void     printStats() const;
        void     printClauseStats() const;
        size_t   getNumFreeVars() const;
        const SolverConf& getConf() const;
        void setConf(SolverConf conf);
        const vector<std::pair<string, string> >& get_tags() const;
        const BinTriStats& getBinTriStats() const;
        size_t   getNumLongIrredCls() const;
        size_t   getNumLongRedCls() const;
        size_t getNumVarsElimed() const;
        size_t getNumVarsReplaced() const;
        Var numActiveVars() const;
        void printMemStats() const;
        uint64_t printWatchMemUsed(uint64_t totalMem) const;
        unsigned long get_sql_id() const;
        const SolveStats& getSolveStats() const;
        void add_in_partial_solving_stats();


        ///Return number of variables waiting to be replaced
        size_t getNewToReplaceVars() const;
        const Stats& getStats() const;
        uint64_t getNextCleanLimit() const;

        ///////////////////////////////////
        // State Dumping
        void dumpRedClauses(
            std::ostream* os
            , const uint32_t maxSize
        ) const;
        void dumpIrredClauses(
            std::ostream* os
        ) const;
        void dump_clauses(
            const vector<ClOffset>& cls
            , std::ostream* os
            , size_t max_size = std::numeric_limits<size_t>::max()
        ) const;
        void dump_blocked_clauses(std::ostream* os) const;
        void dump_component_clauses(std::ostream* os) const;
        void write_irred_stats_to_cnf(std::ostream* os) const;


        //Checks
        void checkImplicitPropagated() const;
        void checkStats(const bool allowFreed = false) const;
        uint64_t countLits(
            const vector<ClOffset>& clause_array
            , bool allowFreed
        ) const;
        void checkImplicitStats() const;
        bool find_with_stamp_a_or_b(Lit a, Lit b) const;
        bool find_with_cache_a_or_b(Lit a, Lit b, int64_t* limit) const;
        bool find_with_watchlist_a_or_b(Lit a, Lit b, int64_t* limit) const;

    protected:
        uint64_t getNumLongClauses() const;
        vector<Lit> finalCl_tmp;
        bool addClause(const vector<Lit>& ps);
        void new_var(bool bva = false, Var orig_outer = std::numeric_limits<Var>::max()) override;
        void new_vars(size_t n) override;

        void set_up_sql_writer();
        SQLStats* sqlStats;
        vector<std::pair<string, string> > sql_tags;

        //Attaching-detaching clauses
        void attachClause(
            const Clause& c
            , const bool checkAttach = true
        ) override;
        void attachBinClause(
            const Lit lit1
            , const Lit lit2
            , const bool red
            , const bool checkUnassignedFirst = true
        ) override;
        void attachTriClause(
            const Lit lit1
            , const Lit lit2
            , const Lit lit3
            , const bool red
        ) override;
        void detachTriClause(
            Lit lit1
            , Lit lit2
            , Lit lit3
            , bool red
            , bool allow_empty_watch = false
        ) override;
        void detachBinClause(
            Lit lit1
            , Lit lit2
            , bool red
            , bool allow_empty_watch = false
        ) override;
        void detachClause(const Clause& c, const bool removeDrup = true);
        void detachClause(const ClOffset offset, const bool removeDrup = true);
        void detachModifiedClause(
            const Lit lit1
            , const Lit lit2
            , const uint32_t origSize
            , const Clause* address
        ) override;
        Clause* addClauseInt(
            const vector<Lit>& lits
            , const bool red = false
            , const ClauseStats stats = ClauseStats()
            , const bool attach = true
            , vector<Lit>* finalLits = NULL
            , bool addDrup = true
            , const Lit drup_first = lit_Undef
        );

    private:
        void check_config_parameters() const;
        void handle_found_solution(const lbool status);
        void add_every_combination_xor(const vector<Lit>& lits, bool attach);
        void add_xor_clause_inter_cleaned_cut(const vector<Lit>& lits, bool attach);
        unsigned num_bits_set(const size_t x, const unsigned max_size) const;
        void check_too_large_variable_number(const vector<Lit>& lits) const;
        void set_assumptions();
        void dumpUnitaryClauses(std::ostream* os) const;
        void dumpEquivalentLits(std::ostream* os) const;
        void dumpBinClauses(
            const bool dumpRed
            , const bool dumpIrred
            , std::ostream* outfile
        ) const;

        void dumpTriClauses(
            const bool alsoRed
            , const bool alsoIrred
            , std::ostream* outfile
        ) const;
        void     open_dump_file(std::ofstream& outfile, std::string filename) const;
        uint64_t count_irred_clauses_for_dump() const;
        struct ReachabilityStats
        {
            ReachabilityStats() :
                cpu_time(0)
                , numLits(0)
                , dominators(0)
                , numLitsDependent(0)
            {}

            ReachabilityStats& operator+=(const ReachabilityStats& other)
            {
                cpu_time += other.cpu_time;

                numLits += other.numLits;
                dominators += other.dominators;
                numLitsDependent += other.numLitsDependent;

                return *this;
            }

            void print() const
            {
                cout << "c ------- REACHABILITY STATS -------" << endl;
                printStatsLine("c time"
                    , cpu_time
                );

                printStatsLine("c dominator lits"
                    , stats_line_percent(dominators, numLits)
                    , "% of unknowns lits"
                );

                printStatsLine("c dependent lits"
                    , stats_line_percent(numLitsDependent, numLits)
                    , "% of unknown lits"
                );

                printStatsLine("c avg num. dominated lits"
                    , (double)numLitsDependent/(double)dominators
                );

                cout << "c ------- REACHABILITY STATS END -------" << endl;
            }

            void printShort() const
            {
                cout
                << "c [reach]"
                << " dom lits: " << std::fixed << std::setprecision(2)
                << stats_line_percent(dominators, numLits)
                << " %"

                << " dep-lits: " << std::fixed << std::setprecision(2)
                << stats_line_percent(numLitsDependent, numLits)
                << " %"

                << " dep-lits/dom-lits : " << std::fixed << std::setprecision(2)
                << stats_line_percent(numLitsDependent, dominators)

                << " T: " << std::fixed << std::setprecision(2)
                << cpu_time << " s"
                << endl;
            }

            double cpu_time;

            size_t numLits;
            size_t dominators;
            size_t numLitsDependent;
        };

        template<class T> vector<Lit> clauseBackNumbered(const T& cl) const;

        lbool solve();
        vector<Lit> back_number_from_caller_tmp;
        template<class T>
        void back_number_from_caller(const vector<T>& lits)
        {
            back_number_from_caller_tmp.clear();
            for (const T& lit: lits) {
                assert(lit.var() < nVarsOutside());
                back_number_from_caller_tmp.push_back(map_to_with_bva(lit));
                assert(back_number_from_caller_tmp.back().var() < nVarsOuter());
            }
        }
        void check_switchoff_limits_newvar(size_t n = 1);
        vector<Lit> origAssumptions;
        void checkDecisionVarCorrectness() const;
        bool enqueueThese(const vector<Lit>& toEnqueue);

        //Stats printing
        void printMinStats() const;
        void printFullStats() const;

        bool add_xor_clause_inter(
            const vector< Lit >& lits
            , bool rhs
            , const bool attach
        );

        lbool simplifyProblem();
        SolveStats solveStats;
        void check_minimization_effectiveness(lbool status);
        void check_recursive_minimization_effectiveness(const lbool status);
        void extend_solution();

        /////////////////////
        // Objects that help us accomplish the task
        friend class ClauseAllocator;
        friend class StateSaver;
        friend class SolutionExtender;
        friend class VarReplacer;
        friend class SCCFinder;
        friend class Prober;
        friend class Vivifier;
        friend class Strengthener;
        friend class Simplifier;
        friend class SubsumeStrengthen;
        friend class ClauseCleaner;
        friend class CompleteDetachReatacher;
        friend class CalcDefPolars;
        friend class ImplCache;
        friend class Searcher;
        friend class XorFinder;
        friend class GateFinder;
        friend class PropEngine;
        friend class CompFinder;
        friend class CompHandler;
        friend class TransCache;
        friend class SubsumeImplicit;
        friend class DataSync;
        Prober              *prober;
        Simplifier          *simplifier;
        SCCFinder           *sCCFinder;
        Vivifier            *vivifier;
        Strengthener        *strengthener;
        ClauseCleaner       *clauseCleaner;
        VarReplacer         *varReplacer;
        CompHandler         *compHandler;
        SubsumeImplicit     *subsumeImplicit;
        DataSync            *datasync = NULL;
        MTRand              mtrand;           ///< random number generator

        /////////////////////////////
        // Temporary datastructs -- must be cleared before use
        mutable std::vector<Lit> tmpCl;

        /////////////////////////////
        //Renumberer
        void renumberVariables();
        void freeUnusedWatches();
        void saveVarMem(uint32_t newNumVars);
        void unSaveVarMem();
        size_t calculate_interToOuter_and_outerToInter(
            vector<Var>& outerToInter
            , vector<Var>& interToOuter
        );
        void renumber_clauses(const vector<Var>& outerToInter);
        void test_renumbering() const;



        /////////////////////////////
        // SAT solution verification
        bool verifyModel() const;
        bool verifyImplicitClauses() const;
        bool verifyClauses(const vector<ClOffset>& cs) const;

        ///////////////////////////
        // Clause cleaning
        void reduce_db_and_update_reset_stats(bool lock_clauses_in = true);
        void clearClauseStats(vector<ClOffset>& clauseset);
        CleaningStats reduceDB(bool lock_clauses_in);
        void lock_most_UIP_used_clauses();
        void lock_in_top_N_uncleaned();
        struct reduceDBStructGlue
        {
            reduceDBStructGlue(ClauseAllocator& _clAllocator) :
                clAllocator(_clAllocator)
            {}
            ClauseAllocator& clAllocator;

            bool operator () (const ClOffset x, const ClOffset y);
        };
        struct reduceDBStructSize
        {
            reduceDBStructSize(ClauseAllocator& _clAllocator) :
                clAllocator(_clAllocator)
            {}
            ClauseAllocator& clAllocator;

            bool operator () (const ClOffset x, const ClOffset y);
        };
        struct reduceDBStructActivity
        {
            reduceDBStructActivity(ClauseAllocator& _clAllocator) :
                clAllocator(_clAllocator)
            {}
            ClauseAllocator& clAllocator;

            bool operator () (const ClOffset x, const ClOffset y);
        };
        struct reduceDBStructPropConfl
        {
            reduceDBStructPropConfl(
                ClauseAllocator& _clAllocator
                , uint64_t _confl_multiplier
            ) :
                clAllocator(_clAllocator)
                , confl_multiplier(_confl_multiplier)
            {}
            ClauseAllocator& clAllocator;
            uint64_t confl_multiplier;

            bool operator () (const ClOffset x, const ClOffset y);
        };
        struct reduceDBStructConflDepth
        {
            reduceDBStructConflDepth(ClauseAllocator& _clAllocator) :
                clAllocator(_clAllocator)
            {}
            ClauseAllocator& clAllocator;

            bool operator () (const ClOffset x, const ClOffset y);
        };
        void pre_clean_clause_db(CleaningStats& tmpStats, uint64_t sumConfl);
        void real_clean_clause_db(
            CleaningStats& tmpStats
            , uint64_t sumConflicts
            , uint64_t removeNum
        );
        uint64_t calc_how_many_to_remove();
        void sort_red_cls_as_required(CleaningStats& tmpStats);
        void print_best_irred_clauses_if_required() const;


        /////////////////////
        // Data
        uint64_t             nextCleanLimit;
        uint64_t             nextCleanLimitInc;
        void setDecisionVar(const uint32_t var);
        void unsetDecisionVar(const uint32_t var);
        size_t               zeroLevAssignsByCNF = 0;
        size_t               zeroLevAssignsByThreads = 0;
        vector<LitReachData> litReachable;
        void calculate_reachability();

        //Main up stats
        Stats sumStats;
        PropStats sumPropStats;
        CleaningStats cleaningStats;
        ReachabilityStats reachStats;

        /////////////////////
        // Clauses
        bool addClauseHelper(vector<Lit>& ps);
        void reArrangeClauses();
        void reArrangeClause(ClOffset offset);
        void printAllClauses() const;
        void consolidateMem();

        //////////////////
        // Stamping
        Lit updateLitForDomin(Lit lit) const;
        void updateDominators();

        /////////////////
        // Debug
        void testAllClauseAttach() const;
        bool normClauseIsAttached(const ClOffset offset) const;
        void findAllAttach() const;
        void findAllAttach(const vector<ClOffset>& cs) const;
        bool findClause(const ClOffset offset) const;
        void checkNoWrongAttach() const;
        void printWatchlist(watch_subarray_const ws, const Lit lit) const;
        void printClauseSizeDistrib();
        ClauseUsageStats sumClauseData(
            const vector<ClOffset>& toprint
            , bool red
        ) const;
        void printPropConflStats(
            std::string name
            , const vector<ClauseUsageStats>& stats
        ) const;
        void check_model_for_assumptions() const;
};

inline void Solver::setDecisionVar(const uint32_t var)
{
    if (!varData[var].is_decision) {
        varData[var].is_decision = true;
        insertVarOrder(var);
    }
}

inline void Solver::unsetDecisionVar(const uint32_t var)
{
    if (varData[var].is_decision) {
        varData[var].is_decision = false;
    }
}

inline uint64_t Solver::getNumLongClauses() const
{
    return longIrredCls.size() + longRedCls.size();
}

inline const Searcher::Stats& Solver::getStats() const
{
    return sumStats;
}

inline uint64_t Solver::getNextCleanLimit() const
{
    return nextCleanLimit;
}

inline const Solver::SolveStats& Solver::getSolveStats() const
{
    return solveStats;
}

inline void Solver::add_sql_tag(const string& tagname, const string& tag)
{
    sql_tags.push_back(std::make_pair(tagname, tag));
}

inline size_t Solver::getNumLongIrredCls() const
{
    return longIrredCls.size();
}

inline size_t Solver::getNumLongRedCls() const
{
    return longRedCls.size();
}

inline const SolverConf& Solver::getConf() const
{
    return conf;
}

inline const vector<std::pair<string, string> >& Solver::get_sql_tags() const
{
    return sql_tags;
}

inline const Solver::BinTriStats& Solver::getBinTriStats() const
{
    return binTri;
}

template<class T>
inline vector<Lit> Solver::clauseBackNumbered(const T& cl) const
{
    tmpCl.clear();
    for(size_t i = 0; i < cl.size(); i++) {
        tmpCl.push_back(map_inter_to_outer(cl[i]));
    }

    return tmpCl;
}

inline Var Solver::numActiveVars() const
{
    Var numActive = 0;
    for(Var var = 0; var < solver->nVars(); var++) {
        if (varData[var].is_decision
            && (varData[var].removed == Removed::none
                || varData[var].removed == Removed::queued_replacer)
            && value(var) == l_Undef
        ) {
            numActive++;
        }
    }

    return numActive;
}

inline lbool Solver::solve_with_assumptions(
    const vector<Lit>* _assumptions
) {
    origAssumptions.clear();
    if (_assumptions) {
        for(const Lit lit: *_assumptions) {
            origAssumptions.push_back(Lit(lit.var(), lit.sign()));
        }
    }
    return solve();
}

inline bool Solver::find_with_stamp_a_or_b(Lit a, const Lit b) const
{
    //start STAMP of A < start STAMP of B
    //end STAMP of A > start STAMP of B
    //means: ~A V B is inside
    //so, invert A
    a = ~a;

    const uint64_t start_inv_other = solver->stamp.tstamp[(a).toInt()].start[STAMP_IRRED];
    const uint64_t start_eqLit = solver->stamp.tstamp[b.toInt()].end[STAMP_IRRED];
    if (start_inv_other < start_eqLit) {
        const uint64_t end_inv_other = solver->stamp.tstamp[(a).toInt()].end[STAMP_IRRED];
        const uint64_t end_eqLit = solver->stamp.tstamp[b.toInt()].end[STAMP_IRRED];
        if (end_inv_other > end_eqLit) {
            return true;
        }
    }

    return false;
}

inline bool Solver::find_with_cache_a_or_b(Lit a, Lit b, int64_t* limit) const
{
    const vector<LitExtra>& cache = solver->implCache[a.toInt()].lits;
    *limit -= cache.size();
    for (LitExtra cacheLit: cache) {
        if (cacheLit.getOnlyIrredBin()
            && cacheLit.getLit() == b
        ) {
            return true;
        }
    }

    std::swap(a,b);

    const vector<LitExtra>& cache2 = solver->implCache[a.toInt()].lits;
    *limit -= cache2.size();
    for (LitExtra cacheLit: cache) {
        if (cacheLit.getOnlyIrredBin()
            && cacheLit.getLit() == b
        ) {
            return true;
        }
    }

    return false;
}

inline bool Solver::find_with_watchlist_a_or_b(Lit a, Lit b, int64_t* limit) const
{
    if (watches[a.toInt()].size() > watches[b.toInt()].size()) {
        std::swap(a,b);
    }

    watch_subarray_const ws = watches[a.toInt()];
    *limit -= ws.size();
    for (const Watched w: ws) {
        if (!w.isBinary())
            continue;

        if (!w.red()
            && w.lit2() == b
        ) {
            return true;
        }
    }

    return false;
}

inline const vector<lbool>& Solver::get_model() const
{
    return model;
}

inline const vector<Lit>& Solver::get_final_conflict() const
{
    return conflict;
}

inline void Solver::setConf(const SolverConf _conf)
{
    conf = _conf;
}

} //end namespace

#endif //SOLVER_H
