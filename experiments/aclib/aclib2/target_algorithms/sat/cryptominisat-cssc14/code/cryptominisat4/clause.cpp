#include "clause.h"

using namespace CMSat;

void ClauseUsageStats::print() const
{
    cout
    #ifdef STATS_NEEDED
    << " lits visit: "
    << std::setw(8) << sumLitVisited/1000UL
    << "K"

    << " cls visit: "
    << std::setw(7) << sumLookedAt/1000UL
    << "K"
    #endif

    << " prop: "
    << std::setw(5) << sumProp/1000UL
    << "K"

    << " conf: "
    << std::setw(5) << sumConfl/1000UL
    << "K"

    << " UIP used: "
    << std::setw(5) << sumUsedUIP/1000UL
    << "K"
    << endl;
}

void CleaningStats::print(const size_t nbReduceDB) const
{
    cout << "c ------ CLEANING STATS ---------" << endl;
    //Pre-clean
    printStatsLine("c pre-removed"
        , preRemove.num
        , stats_line_percent(preRemove.num, origNumClauses)
        , "% long redundant clauses"
    );

    printStatsLine("c pre-removed lits"
        , preRemove.lits
        , stats_line_percent(preRemove.lits,origNumLits)
        , "% long red lits"
    );
    printStatsLine("c pre-removed cl avg size"
        , (double)preRemove.lits/(double)preRemove.num
    );
    printStatsLine("c pre-removed cl avg glue"
        , (double)preRemove.glue/(double)preRemove.num
    );
    printStatsLine("c pre-removed cl avg num resolutions"
        , (double)preRemove.sumResolutions()/(double)preRemove.num
    );

    //Types of clean
    printStatsLine("c clean by glue"
        , glueBasedClean
        , stats_line_percent(glueBasedClean, nbReduceDB)
        , "% cleans"
    );
    printStatsLine("c clean by size"
        , sizeBasedClean
        , stats_line_percent(sizeBasedClean, nbReduceDB)
        , "% cleans"
    );
    printStatsLine("c clean by prop&confl"
        , propConflBasedClean
        , stats_line_percent(propConflBasedClean,nbReduceDB)
        , "% cleans"
    );

    //--- Actual clean --

    //-->CLEAN
    printStatsLine("c cleaned cls"
        , removed.num
        , stats_line_percent(removed.num, origNumClauses)
        , "% long redundant clauses"
    );
    printStatsLine("c cleaned lits"
        , removed.lits
        , stats_line_percent(removed.lits, origNumLits)
        , "% long red lits"
    );
    printStatsLine("c cleaned cl avg size"
        , (double)removed.lits/(double)removed.num
    );
    printStatsLine("c cleaned avg glue"
        , (double)removed.glue/(double)removed.num
    );

    //--> REMAIN
    printStatsLine("c remain cls"
        , remain.num
        , stats_line_percent(remain.num, origNumClauses)
        , "% long redundant clauses"
    );
    printStatsLine("c remain lits"
        , remain.lits
        , stats_line_percent(remain.lits, origNumLits)
        , "% long red lits"
    );
    printStatsLine("c remain cl avg size"
        , (double)remain.lits/(double)remain.num
    );
    printStatsLine("c remain avg glue"
        , (double)remain.glue/(double)remain.num
    );

    cout << "c ------ CLEANING STATS END ---------" << endl;
}

void CleaningStats::printShort() const
{
    //Pre-clean
    cout
    << "c [DBclean]"
    << " Pre-removed: "
    << preRemove.num
    << " clean type will be " << getNameOfCleanType(clauseCleaningType)
    << endl;

    cout
    << "c [DBclean]"
    << " rem " << removed.num

    << " avgGlue " << std::fixed << std::setprecision(2)
    << ((double)removed.glue/(double)removed.num)

    << " avgSize "
    << std::fixed << std::setprecision(2)
    << ((double)removed.lits/(double)removed.num)
    << endl;

    cout
    << "c [DBclean]"
    << " remain " << remain.num

    << " avgGlue " << std::fixed << std::setprecision(2)
    << ((double)remain.glue/(double)remain.num)

    << " avgSize " << std::fixed << std::setprecision(2)
    << ((double)remain.lits/(double)remain.num)

    << " T " << std::fixed << std::setprecision(2)
    << cpu_time
    << endl;
}
