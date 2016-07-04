
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>


class MockOntology;

std::ostream& operator<<(std::ostream& out, const MockOntology& ontology);



class MockOntology
{
public:
    typedef std::string concept_type;
    typedef std::string relation_type;


    typedef std::vector<concept_type> Concepts;
    typedef std::map<concept_type,concept_type> ConceptPair;
    typedef std::map<relation_type, ConceptPair> Relations;

private:
    Concepts  m_concepts;
    Relations m_relations;


public:


    MockOntology () {}

    void addConcept (std::string concept) { m_concepts.push_back (concept); }
    void addRelation (std::string startConcept, std::string relation, std::string targetConcept) 
    { 
	Relations::iterator itRelation = m_relations.find (relation);
	if (itRelation == m_relations.end())
	{
	    std::pair<Relations::iterator,bool> insertResult = m_relations.insert (Relations::value_type(relation,ConceptPair()));
	    if (!insertResult.second)
		throw std::string("ConceptPair could not be inserted into ontology relation");
	    itRelation = insertResult.first;
	}
	itRelation->second.insert (ConceptPair::value_type(startConcept, targetConcept));
    }


    template <typename Iterator>
    concept_type getTargetConceptForPath (const concept_type& startConcept, Iterator itRelation, Iterator itRelationEnd)
    {
	if (itRelation == itRelationEnd)
	    return startConcept;
	concept_type concept = startConcept;
	while (itRelation != itRelationEnd)
	{
	    concept = m_relations[(*itRelation)][concept];
	    ++itRelation;
	}
	return concept;
    }


    friend std::ostream& operator<<(std::ostream& out, const MockOntology& ontology);
};




std::ostream& operator<<(std::ostream& out, const MockOntology& ontology)
{
    out << "--- CONCEPTS ---" << std::endl;
    for (MockOntology::Concepts::const_iterator itConcept = ontology.m_concepts.begin(), itConceptEnd = ontology.m_concepts.end(); itConcept != itConceptEnd; ++itConcept)
    {
	out << (*itConcept) << ", ";
    }
    out << std::endl;

    out << "--- RELATIONS ---" << std::endl;
    for (MockOntology::Relations::const_iterator itRelation = ontology.m_relations.begin(), itRelationEnd = ontology.m_relations.end(); itRelation != itRelationEnd; ++itRelation)
    {
	const MockOntology::relation_type& relation = itRelation->first;
	for (MockOntology::ConceptPair::const_iterator itCP = itRelation->second.begin(), itCPEnd = itRelation->second.end(); itCP != itCPEnd; ++itCP)
	{
	    out << (itCP->first) << " --> " << relation << " --> " << (itCP->second) << std::endl;
	}
    }
    out << std::endl;
    return out;
}






class MockDataAccessor
{
public:
};






void test_MockOntology ()
{
    MockOntology ontology; 
    ontology.addConcept ("Person");
    ontology.addConcept ("Book");

    ontology.addRelation ("Person", "isAuthorOf", "Book");
    ontology.addRelation ("Book", "isEditedBy", "Person");

    std::cout << ontology;


    MockOntology::concept_type concept("Person");
    std::vector<MockOntology::relation_type> relationPath;

    std::cout << "Person: " << ontology.getTargetConceptForPath (concept, relationPath.begin(), relationPath.end()) << std::endl;
    relationPath.push_back ("isAuthorOf");
    std::cout << "Person/isAuthorOf: " << ontology.getTargetConceptForPath (concept, relationPath.begin(), relationPath.end()) << std::endl;
    relationPath.push_back ("isEditedBy");
    std::cout << "Person/isAuthorOf/isEditedBy: " << ontology.getTargetConceptForPath (concept, relationPath.begin(), relationPath.end()) << std::endl;

    std::cout << std::endl;
}



int main ()
{
    test_MockOntology ();
}


