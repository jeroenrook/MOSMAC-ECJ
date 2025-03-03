// TODO: This is now useless, remove it

#ifndef GGAEVALUATIONSTATISTICS_HPP
#define	GGAEVALUATIONSTATISTICS_HPP

#include <boost/serialization/access.hpp>

class GGAEvaluationStatistics 
{
public:
    GGAEvaluationStatistics();
    GGAEvaluationStatistics(const GGAEvaluationStatistics& o);
    virtual ~GGAEvaluationStatistics();

    GGAEvaluationStatistics& operator=(const GGAEvaluationStatistics& o);
    
    void addEvaluation(double performance);    
    void addEvaluation(const GGAEvaluationStatistics&);
    
    double getAvgPerformance() const;
    unsigned getNumberOfEvaluations() const;
private:
    // serialization
    friend class boost::serialization::access;
    template <class Archiver> void serialize(Archiver&, const unsigned int);
    
    // attributes    
    double m_avg_performance;
    unsigned m_nevaluations;
};

// [public in-line] ------------------------------------------------------------

inline double GGAEvaluationStatistics::getAvgPerformance() const
{
    return m_avg_performance;
}

inline unsigned GGAEvaluationStatistics::getNumberOfEvaluations() const
{
    return m_nevaluations;
}

// [serialization] -------------------------------------------------------------

template<typename Archiver>
void GGAEvaluationStatistics::serialize(Archiver& ar, const unsigned int version)
{   
    ar & m_avg_performance;
    ar & m_nevaluations;
}


#endif	/* GGAEVALUATIONSTATISTICS_H */

