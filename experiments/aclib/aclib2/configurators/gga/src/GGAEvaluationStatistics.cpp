// TODO: This is now useless, remove it

#include "GGAEvaluationStatistics.hpp"

// --------------------- //
// GGAInstanceStatistics //
// --------------------- //

GGAEvaluationStatistics::GGAEvaluationStatistics()
    : m_avg_performance(0.0)
    , m_nevaluations(0)
{ }

GGAEvaluationStatistics::GGAEvaluationStatistics(const GGAEvaluationStatistics& o)
    : m_avg_performance(o.m_avg_performance)
    , m_nevaluations(o.m_nevaluations)
{ }

GGAEvaluationStatistics::~GGAEvaluationStatistics()
{ }

GGAEvaluationStatistics& GGAEvaluationStatistics::operator =(
        const GGAEvaluationStatistics& o)
{
    m_avg_performance = o.m_avg_performance;
    m_nevaluations = o.m_nevaluations;
    return *this;
}

void GGAEvaluationStatistics::addEvaluation(double performance)
{
    if (m_nevaluations > 0)
        m_avg_performance = (m_avg_performance + performance) / 2;
    else
        m_avg_performance = performance;
    m_nevaluations += 1;
}

void GGAEvaluationStatistics::addEvaluation(
        const GGAEvaluationStatistics& evals)
{
    if (m_nevaluations > 0)
        m_avg_performance = (m_avg_performance + evals.m_avg_performance) / 2;
    else
        m_avg_performance = evals.m_avg_performance;
    m_nevaluations += evals.m_nevaluations;
}
