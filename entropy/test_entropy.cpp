

#include <math.h>
#include <iostream>
#include <iterator>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cassert>

#include <fenv.h>



//#define DBG(x) x
#define DBG(x) 




struct DataIndex
{

    DataIndex (size_t _index, int _minCategory, int _numCategories)
        : index (_index)
        , minCategory (_minCategory)
        , numCategories (_numCategories)
    { 
    }

    bool operator< (const DataIndex& other) const { return index < other.index; }

    size_t index;
    int minCategory;
    int numCategories;
};


struct CombinationFeature
{
    CombinationFeature (int _index0, 
                        int _index1, 
                        int _feature0MinCategory,
                        int _feature1MinCategory, 
                        int _feature0NumCategories, 
                        int _feature1NumCategories,
                        double _in01_out2, 
                        double _joint02, 
                        double _joint12)
        : index0(_index0)
        , index1(_index1)
        , feature0MinCategory (_feature0MinCategory)
        , feature1MinCategory (_feature1MinCategory)
        , feature0NumCategories (_feature0NumCategories)
        , feature1NumCategories (_feature1NumCategories)
        , in01_out2 (_in01_out2)
        , joint02 (_joint02)
        , joint12 (_joint12)
    {}


    // compute the output feature category from the combination feature and the two featureCategories from the combination feature
    int getComboOutput (int feature0Category, int feature1Category) const
    {
        int feature0CategoryIndex = feature0Category - feature0MinCategory;
        int feature1CategoryIndex = feature1Category - feature1MinCategory;

        return mapping.at (feature0NumCategories * feature1CategoryIndex + feature0CategoryIndex); // output feature category
    }



    int index0;
    int index1;


    int feature0MinCategory;
    int feature1MinCategory;
    
    int feature0NumCategories;
    int feature1NumCategories;

    double in01_out2;
    double joint02;
    double joint12;

    std::vector<int> mapping;
};




class MutualInformation
{
public:

    MutualInformation (const std::vector<std::vector<int> >& _data, const std::vector<double>& _weights)
        : m_features (_data)
        , m_weights (_weights)
    {
        if (m_features.empty ())
            return;

        m_numFeatures = m_features.size ();
        std::vector<std::pair<int, int> > minMax;
        minMax.assign (m_numFeatures, std::make_pair(0,0));
        std::vector<std::pair<int,int> >::iterator itMM = minMax.begin ();
        for (std::vector<std::vector<int> >::const_iterator it = m_features.begin (), itEnd = m_features.end (); it != itEnd; ++it, ++itMM)
        {
            const std::vector<int>& features = (*it);
            for (std::vector<int>::const_iterator it = features.begin (), itEnd = features.end (); it != itEnd; ++it)
            {
                const int& val = (*it);
                std::pair<int,int>& mm = (*itMM);
                mm.first = std::min (mm.first, val);
                mm.second = std::max (mm.second, val);
            }
        }

        int idx = 0;
        for (std::vector<std::pair<int,int> >::const_iterator it = minMax.begin (), itEnd = minMax.end (); it != itEnd; ++it)
        {
            m_dataIndices.push_back (DataIndex (idx, (*it).first, (*it).second - (*it).first+1));
            ++idx;
        }

        m_Hx.assign (m_numFeatures, 0.0);
        m_H_joint_xy.assign (m_numFeatures*m_numFeatures, 0.0);
        m_H_joint_xyz.assign (m_numFeatures*m_numFeatures*m_numFeatures, 0.0);

        m_isHx.assign (m_numFeatures, false);
        m_isH_joint_xy.assign (m_numFeatures*m_numFeatures, false);
        m_isH_joint_xyz.assign (m_numFeatures*m_numFeatures*m_numFeatures, false);

    }


    double H_x (int idx) 
    {
        int index = indexOf (m_dataIndices.at (idx));
        if (!m_isHx.at (index))
            compute_H_x (m_dataIndices.at (index));
        return m_Hx.at (index);
    }

    double H_joint_xy (int idx0, int idx1) 
    {
        int index = indexOf (m_dataIndices.at (idx0), m_dataIndices.at (idx1));
        if (!m_isH_joint_xy.at (index))
            compute_H_xy (m_dataIndices.at (idx0), m_dataIndices.at (idx1));
        return m_H_joint_xy.at (index);
    }

    double H_joint_xyz (int idx0, int idx1, int idx2) 
    {
        int index = indexOf (m_dataIndices.at (idx0), m_dataIndices.at (idx1), m_dataIndices.at (idx2));
        if (!m_isH_joint_xyz.at (index))
            compute_H_xyz (m_dataIndices.at (idx0), m_dataIndices.at (idx1), m_dataIndices.at (idx2));
        return m_H_joint_xyz.at (index);
    }


    // mutual informations
    double I_joint_xy (int idx0, int idx1)
    {
        double h_xy = H_joint_xy (idx0, idx1);
        double h_x  = H_x (idx0);
        double h_y  = H_x (idx1);
        return h_x + h_y - h_xy;
    }


    double H_x_given_y (int idx0, int idx1)
    {
        double h_xy = H_joint_xy (idx0, idx1);
        double h_x  = H_x (idx1);
        return h_xy - h_x;
    }

    double H_x_given_yz (int idx0, int idx1, int idx2)
    {
        double h_x_given_z = H_x_given_y (idx0, idx1);
        double h_joint_xz = H_joint_xy (idx0, idx2);
        double h_joint_yz = H_joint_xy (idx1, idx2);
        double h_joint_xyz = H_joint_xyz (idx0, idx1, idx2);
        double h_z  = H_x (idx2);
        return h_x_given_z - h_joint_xz - h_joint_yz + h_joint_xyz + h_z;
    }

    double H_xy_given_z (int idx0, int idx1, int idx2)
    {
        double h_y_given_xz = H_x_given_yz(idx1, idx0, idx2);
        double h_x_given_z  = H_x_given_y (idx0, idx2);
        return h_y_given_xz + h_x_given_z;
    }



    double I_xy_given_z (int idx0, int idx1, int idx2)
    {
        double h_xyz = H_joint_xyz (idx0, idx1, idx2); // H(X,Y,Z)
        double h_xz  = H_joint_xy  (idx0, idx2);       // H(X,Z)
        double h_yz  = H_joint_xy  (idx1, idx2);       // H(Y,Z)
        double h_z   = H_x         (idx2);             // H(Z)

        return h_xz + h_yz - h_xyz - h_z;
    }
    
    double I_joint_xyz (int idx0, int idx1, int idx2)
    {
        double i_xy_given_z = I_xy_given_z (idx0, idx1, idx2);
        double i_xz = I_joint_xy (idx0, idx2);
        return i_xz + i_xy_given_z;
    }

    double I_int_xyz (int idx0, int idx1, int idx2)
    {
        double i_xy_given_z = I_xy_given_z (idx0, idx1, idx2);
        double i_xy = I_joint_xy (idx0, idx1);
        return i_xy_given_z - i_xy;
    }

    double I_in_xy_out_z (int idx0, int idx1, int idx2) // == I_in_x_out_xz
    {
        double h_joint_xyz = H_joint_xyz (idx0, idx1, idx2);
        double h_xy = H_joint_xy (idx0, idx1);
        double h_z  = H_x (idx2);
        return h_xy + h_z - h_joint_xyz;
    }




    std::vector<CombinationFeature> combinationFeatures ()
    {
        int featuresEnd = m_numFeatures-1;
        int outputFeature = featuresEnd;
        std::vector<CombinationFeature> featureIndices;
        for (int i = 0; i < featuresEnd; ++i)
        {
            if (H_x (i) < 0.0001)
                continue;
            for (int j = i+1; j < featuresEnd; ++j)
            {
                if (H_x (j) < 0.0001)
                    continue;
                double i_in_01_out_2  = I_in_xy_out_z (i,j,outputFeature);
                double i_joint_02  = I_joint_xy (i,outputFeature);
                double i_joint_12  = I_joint_xy (j,outputFeature);
                bool decide = (i_in_01_out_2 > i_joint_02 && i_in_01_out_2 > i_joint_12);
                if (decide)
                {
                    int feature0MinCategory = m_dataIndices.at (i).minCategory;
                    int feature0NumCategories = m_dataIndices.at (i).numCategories;
                    int feature1MinCategory = m_dataIndices.at (j).minCategory;
                    int feature1NumCategories = m_dataIndices.at (j).numCategories;
                    featureIndices.push_back (CombinationFeature(i,j,
                                                                 feature0MinCategory,
                                                                 feature1MinCategory,
                                                                 feature0NumCategories,
                                                                 feature1NumCategories,
                                                                 i_in_01_out_2,i_joint_02,i_joint_12));
                }
            }
        }
        return featureIndices;
    }


    

    void computeCombinationFeatureMapping (CombinationFeature& combo)
    {
        DataIndex& feature0 = m_dataIndices.at (combo.index0);
        DataIndex& feature1 = m_dataIndices.at (combo.index1);
        DataIndex& outputFeature = m_dataIndices.back ();
        

        std::vector<int> feature0Vector = m_features.at (feature0.index);
        std::vector<int> feature1Vector = m_features.at (feature1.index);
        std::vector<int> feature2Vector = m_features.at (outputFeature.index);
        std::vector<double>::const_iterator itWeight = m_weights.begin ();

        int numFeature0Categories = feature0.numCategories;
        int numFeature1Categories = feature1.numCategories;
        int numFeature2Categories = outputFeature.numCategories;

        size_t numMappings = numFeature0Categories * numFeature1Categories * numFeature2Categories;
        std::vector<double> mapping (numMappings, 0.0);
        double sumOfWeights (0.0);

        std::vector<int>::const_iterator itF0 = feature0Vector.begin ();
        std::vector<int>::const_iterator itF0End = feature0Vector.end ();
        std::vector<int>::const_iterator itF1 = feature1Vector.begin ();
        std::vector<int>::const_iterator itF2 = feature2Vector.begin ();
        for (; itF0 != itF0End; ++itF0, ++itF1, ++itF2, ++itWeight)
        {
            const double& weight = (*itWeight);
            sumOfWeights += weight;

            const int& feature0Category = (*itF0);
            const int& feature1Category = (*itF1);
            const int& feature2Category = (*itF2);
            int feature0Index = feature0Category + feature0.minCategory;
            int feature1Index = feature1Category + feature1.minCategory;
            int feature2Index = feature2Category + outputFeature.minCategory;

            int pos = numFeature0Categories * numFeature1Categories * feature2Index +  
                      numFeature0Categories * feature1Index + 
                      feature0Index;
            mapping.at (pos) += weight;
        }
        

        // compute most probable output  
        double largest = 0.0;
        int output = -2;
        for (int idxF2 = 0; idxF2 < numFeature2Categories; ++idxF2)
        {
            double pSum = 0.0;
            for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
            {
                for (int idxF1 = 0; idxF1 < numFeature1Categories; ++idxF1)
                {
                    int pos = numFeature0Categories * numFeature1Categories * idxF2 +  
                        numFeature0Categories * idxF1 + 
                        idxF0;
                    pSum += mapping.at (pos);
                }
            }
            if (pSum > largest)
            {
                largest = pSum;
                output = idxF2 + outputFeature.minCategory;
            }
        }


        combo.mapping.assign (numFeature0Categories * numFeature1Categories, -2);
        for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
        {
            for (int idxF1 = 0; idxF1 < numFeature1Categories; ++idxF1)
            {
                int tgtPos = numFeature0Categories * idxF1 + idxF0;
                largest = -0.1;
                // find the best match
                for (int idxF2 = 0; idxF2 < numFeature2Categories; ++idxF2)
                {
                    int pos = numFeature0Categories * numFeature1Categories * idxF2 +  
                        numFeature0Categories * idxF1 + 
                        idxF0;
                    int m = mapping.at (pos);
                    if (m > largest)
                    {
                        largest = m;
                        output = idxF2 + outputFeature.minCategory;
                    }
                }
                combo.mapping.at (tgtPos) = output;
            }
        }
    }


    // // compute the output feature category from the combination feature and the two featureCategories from the combination feature
    // int combinationFeatureOutput (const CombinationFeature& combo, int feature0Category, int feature1Category)
    // {
    //     const DataIndex& feature0Index = m_dataIndices.at (combo.index0);
    //     const DataIndex& feature1Index = m_dataIndices.at (combo.index1);

    //     int feature0CategoryIndex = feature0Category - feature0Index.minCategory;
    //     int feature1CategoryIndex = feature1Category - feature1Index.minCategory;

    //     return combo.mapping.at (feature0Index.numCategories * feature1CategoryIndex + feature0CategoryIndex); // output feature category
    // }





    std::vector<int> cmim ()
    {
        int featuresEnd = m_numFeatures-1;
        int outputFeature = featuresEnd;
        std::vector<double> partialScores (featuresEnd, 0.0);
        std::vector<int> lastPicked (featuresEnd, 0);
        std::vector<int> nonAlreadyUpdated (featuresEnd, 0);
        for (int n = 0; n < featuresEnd; ++n)
        {
            partialScores.at (n) = I_joint_xy (n, outputFeature);
        }
        for (int k = 0; k < featuresEnd; ++k)
        {
            double score = 0.0;
            for (int n = 0; n < featuresEnd; ++n)
            {
                while (partialScores.at (n) > score && lastPicked.at (n) < k-1)
                {
                    lastPicked.at (n) += 1;
                    partialScores.at (n) = std::min (partialScores.at (n), I_xy_given_z (outputFeature, n, nonAlreadyUpdated.at(lastPicked.at (n))));
                }
                if (partialScores.at (n) > score)
                {
                    score = partialScores.at (n);
                    nonAlreadyUpdated.at (k) = n;
                }
            }
        }
        std::ostream_iterator<double> out_it (std::cout, " ,  ");
        std::copy (partialScores.begin (), partialScores.end (), out_it); 
        return nonAlreadyUpdated;
    }




protected:


    void compute_H_x (DataIndex index0) 
    {
        std::vector<int> feature0Vector = m_features.at (index0.index);
        std::vector<double>::const_iterator itWeight = m_weights.begin ();

        int numFeature0Categories = index0.numCategories;

        int numCounts = numFeature0Categories;
        std::vector<double> counts; 
        counts.assign (numCounts, 0.0);
        double sumOfWeights = 0.0;
        for (std::vector<int>::const_iterator itF0 = feature0Vector.begin (), itF0End = feature0Vector.end (); 
             itF0 != itF0End; ++itF0, ++itWeight)
        {
            const double& weight = (*itWeight);
            sumOfWeights += weight;

            const int& feature0Category = (*itF0);
            int feature0Index = feature0Category + index0.minCategory;

            int pos = feature0Index;
            counts.at (pos) += weight;
        }

        double H_0 = 0.0;

        DBG(std::cout << std::endl;)
        DBG(std::cout << " --- H_x (" << index0.index << ") " << std::endl;)
        for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
        {
            DBG(std::cout << "F_0:" << idxF0 << " : ";)
            double p = counts.at (idxF0)/sumOfWeights;
            DBG(std::cout << std::setw (5) << p; )
            double h_0 = - xi (p);
            H_0 += h_0;
            DBG(std::cout << "  | h_0 = " << h_0 << std::endl;)
        }
        DBG(std::cout << "  H_0 = " << H_0 << std::endl;)


        m_Hx.at (indexOf (index0)) = H_0;
        m_isHx.at (indexOf (index0)) = true;
    }




    void compute_H_xy (DataIndex index0, DataIndex index1) 
    {
        std::vector<int> feature0Vector = m_features.at (index0.index);
        std::vector<int> feature1Vector = m_features.at (index1.index);
        std::vector<double>::const_iterator itWeight = m_weights.begin ();

        int numFeature0Categories = index0.numCategories;
        int numFeature1Categories = index1.numCategories;

        int numCounts = numFeature0Categories * numFeature1Categories;
        std::vector<double> counts; 
        counts.assign (numCounts, 0.0);
        double sumOfWeights = 0.0;

        std::vector<int>::const_iterator itF0 = feature0Vector.begin ();
        std::vector<int>::const_iterator itF0End = feature0Vector.end ();
        std::vector<int>::const_iterator itF1 = feature1Vector.begin ();
        for (; itF0 != itF0End; ++itF0, ++itF1,++itWeight)
        {
            const double& weight = (*itWeight);
            sumOfWeights += weight;

            const int& feature0Category = (*itF0);
            const int& feature1Category = (*itF1);
            int feature0Index = feature0Category + index0.minCategory;
            int feature1Index = feature1Category + index1.minCategory;

            int pos = numFeature0Categories * feature1Index + feature0Index;
            counts.at (pos) += weight;
        }

        double H_0 = 0.0;
        double H_1 = 0.0;
        double H_j_01 = 0.0;

        DBG(std::cout << std::endl;);
        DBG(std::cout << " --- H_xy (" << index0.index << ", " << index1.index << ") " << std::endl;);
        for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
        {
            double p_0 = 0.0;
            DBG(std::cout << "F_0:" << idxF0 << " : ";);
            for (int idxF1 = 0; idxF1 < numFeature1Categories; ++idxF1)
            {
                int pos = numFeature0Categories * idxF1 + idxF0;
                double p = counts.at (pos)/sumOfWeights;
                DBG(std::cout << " f_1:" << idxF1 << "=" << std::setw (5) << p; );

                p_0 += p;
                double h = -xi(p);
                H_j_01 += h;
            }
            double h_0 = - xi (p_0);
            H_0 += h_0;
            DBG(std::cout << "   h_0 = " << h_0 << std::endl;);
        }
        DBG(std::cout << "  H_0 = " << H_0 << std::endl;);
        DBG(std::cout << "  H_j_01 = " << H_j_01 << std::endl;);
        DBG(std::cout << std::endl;);

        for (int idxF1 = 0; idxF1 < numFeature1Categories; ++idxF1)
        {
            double p_1 = 0.0;
            DBG(std::cout << "F_1:" << idxF1 << " : ";);
            for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
            {
                int pos = numFeature0Categories * idxF1 + idxF0;
                double p = counts.at (pos)/sumOfWeights;
                DBG(std::cout << " f_0:" << idxF0 << "="  << std::setw (5) << p; );
                
                p_1 += p;
            }
            double h_1 = - xi (p_1);
            DBG(std::cout << "   h_1 = " << h_1 << std::endl;);
            H_1 += h_1;
        }
        DBG(std::cout << " H_1 = " << H_1 << std::endl;);
        DBG(std::cout << std::endl;);
        DBG(std::cout << std::endl;);

        m_Hx.at (indexOf (index0)) = H_0;
        m_Hx.at (indexOf (index1)) = H_1;
        m_H_joint_xy.at (indexOf (index0, index1)) = H_j_01;
        m_H_joint_xy.at (indexOf (index1, index0)) = H_j_01;

        m_isHx.at (indexOf (index0)) = true;
        m_isHx.at (indexOf (index1)) = true;
        m_isH_joint_xy.at (indexOf (index0, index1)) = true;
        m_isH_joint_xy.at (indexOf (index1, index0)) = true;
    }







    void compute_H_xyz (DataIndex index0, DataIndex index1, DataIndex index2) 
    {
        std::vector<int> feature0Vector = m_features.at (index0.index);
        std::vector<int> feature1Vector = m_features.at (index1.index);
        std::vector<int> feature2Vector = m_features.at (index2.index);
        std::vector<double>::const_iterator itWeight = m_weights.begin ();

        int numFeature0Categories = index0.numCategories;
        int numFeature1Categories = index1.numCategories;
        int numFeature2Categories = index2.numCategories;

        size_t numCounts = numFeature0Categories * numFeature1Categories * numFeature2Categories;
        std::vector<double> counts; 
        counts.assign (numCounts, 0.0);
        double sumOfWeights = 0.0;

        std::vector<int>::const_iterator itF0 = feature0Vector.begin ();
        std::vector<int>::const_iterator itF0End = feature0Vector.end ();
        std::vector<int>::const_iterator itF1 = feature1Vector.begin ();
        std::vector<int>::const_iterator itF2 = feature2Vector.begin ();
        for (; itF0 != itF0End; ++itF0, ++itF1, ++itF2, ++itWeight)
        {
            const double& weight = (*itWeight);
            sumOfWeights += weight;

            const int& feature0Category = (*itF0);
            const int& feature1Category = (*itF1);
            const int& feature2Category = (*itF2);
            int feature0Index = feature0Category + index0.minCategory;
            int feature1Index = feature1Category + index1.minCategory;
            int feature2Index = feature2Category + index2.minCategory;

            int pos = numFeature0Categories * numFeature1Categories * feature2Index +  
                      numFeature0Categories * feature1Index + 
                      feature0Index;
            counts.at (pos) += weight;
        }

        
        double H_0 = 0.0;
        double H_1 = 0.0;
        double H_2 = 0.0;
        double H_j_01 = 0.0;
        double H_j_12 = 0.0;
        double H_j_02 = 0.0;
        double H_j_012 = 0.0;

        DBG(std::cout << std::endl;);
        DBG(std::cout << " --- H_xyz (" << index0.index << ", " << index1.index << ", " << index2.index << ") " << std::endl;);
        for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
        {
            DBG(std::cout << "F_0::" << idxF0 << " : ";);
            double p_0 = 0.0;
            for (int idxF1 = 0; idxF1 < numFeature1Categories; ++idxF1)
            {
                double p_01 = 0.0;
                for (int idxF2 = 0; idxF2 < numFeature2Categories; ++idxF2)
                {
                    int pos = numFeature0Categories *  numFeature1Categories * idxF2 +  
                        numFeature0Categories * idxF1 + 
                        idxF0;
                    double p = counts.at (pos)/sumOfWeights;
                    DBG(std::cout << " f_12:" << idxF1 << idxF2 << "="  << std::setw (5) << p; );
                
                    p_0 += p;
                    p_01 += p;
                    double h = - xi (p);
                    H_j_012 += h;
                }
                double h_01 = - xi (p_01);
                DBG(std::cout << "   h_01 = " << h_01 << std::endl;);
                H_j_01 += h_01;
            }
            double h_0 = - xi (p_0);
            DBG(std::cout << "   h_0 = " << h_0 << std::endl;);
            H_0 += h_0;
        }
        DBG(std::cout << "  H_0 = " << H_0 << std::endl;);
        DBG(std::cout << "  H_j_01 = " << H_j_01 << std::endl;);
        DBG(std::cout << "  H_j_012 = " << H_j_012 << std::endl;);
        DBG(std::cout << std::endl;);

        for (int idxF1 = 0; idxF1 < numFeature1Categories; ++idxF1)
        {
            DBG(std::cout << "F_1::" << idxF1 << " : ";);
            double p_1 = 0.0;
            for (int idxF2 = 0; idxF2 < numFeature2Categories; ++idxF2)
            {
                double p_12 = 0.0;
                for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
                {
                    int pos = numFeature0Categories *  numFeature1Categories * idxF2 +  
                        numFeature0Categories * idxF1 + 
                        idxF0;
                    double p = counts.at (pos)/sumOfWeights;
                    DBG(std::cout << " f_20:" << idxF2 << idxF0 << "="  << std::setw (5) << p; );
                
                    p_1 += p;
                    p_12 += p;
                }
                double h_12 = - xi (p_12);
                DBG(std::cout << "   h_12 = " << h_12 << std::endl;);
                H_j_12 += h_12;
            }
            double h_1 = - xi (p_1);
            DBG(std::cout << "   h_1 = " << h_1 << std::endl;);
            H_1 += h_1;
        }
        DBG(std::cout << "  H_1 = " << H_1 << std::endl;);
        DBG(std::cout << "  H_j_12 = " << H_j_12 << std::endl;);
        DBG(std::cout << std::endl;);

        for (int idxF2 = 0; idxF2 < numFeature2Categories; ++idxF2)
        {
            DBG(std::cout << "F_2:" << idxF2 << " : ";);
            double p_2 = 0.0;
            for (int idxF0 = 0; idxF0 < numFeature0Categories; ++idxF0)
            {
                double p_02 = 0.0;
                for (int idxF1 = 0; idxF1 < numFeature1Categories; ++idxF1)
                {
                    int pos = numFeature0Categories *  numFeature1Categories * idxF2 +  
                        numFeature0Categories * idxF1 + 
                        idxF0;
                    double p = counts.at (pos)/sumOfWeights;
                    DBG(std::cout << " f_01:" << idxF2 << idxF0 << "="  << std::setw (5) << p; );
                
                    p_2 += p;
                    p_02 += p;
                }
                double h_02 = - xi (p_02);
                DBG(std::cout << "   h_02 = " << h_02 << std::endl;);
                H_j_02 += - xi (p_02);
            }
            double h_2 = - xi (p_2);
            DBG(std::cout << "   h_2 = " << h_2 << std::endl;);
            H_2 += h_2;
        }
        DBG(std::cout << "  H_2 = " << H_2 << std::endl;);
        DBG(std::cout << "  H_j_02 = " << H_j_02 << std::endl;);
        DBG(std::cout << std::endl;);


        m_Hx.at (indexOf (index0)) = H_0;
        m_Hx.at (indexOf (index1)) = H_1;
        m_Hx.at (indexOf (index2)) = H_2;

        m_H_joint_xy.at (indexOf (index0, index1)) = H_j_01;
        m_H_joint_xy.at (indexOf (index1, index0)) = H_j_01;
        m_H_joint_xy.at (indexOf (index0, index2)) = H_j_02;
        m_H_joint_xy.at (indexOf (index2, index0)) = H_j_02;
        m_H_joint_xy.at (indexOf (index1, index2)) = H_j_12;
        m_H_joint_xy.at (indexOf (index2, index1)) = H_j_12;

        m_H_joint_xyz.at (indexOf (index0, index1, index2)) = H_j_012;
        m_H_joint_xyz.at (indexOf (index1, index2, index0)) = H_j_012;
        m_H_joint_xyz.at (indexOf (index2, index0, index1)) = H_j_012;
        m_H_joint_xyz.at (indexOf (index0, index2, index1)) = H_j_012;
        m_H_joint_xyz.at (indexOf (index2, index1, index0)) = H_j_012;
        m_H_joint_xyz.at (indexOf (index1, index0, index2)) = H_j_012;

        m_isHx.at (indexOf (index0)) = true;
        m_isHx.at (indexOf (index1)) = true;
        m_isHx.at (indexOf (index2)) = true;
        m_isH_joint_xy.at (indexOf (index0, index1)) = true;
        m_isH_joint_xy.at (indexOf (index1, index0)) = true;
        m_isH_joint_xy.at (indexOf (index2, index0)) = true;
        m_isH_joint_xy.at (indexOf (index0, index2)) = true;
        m_isH_joint_xy.at (indexOf (index1, index2)) = true;
        m_isH_joint_xy.at (indexOf (index2, index1)) = true;
        m_isH_joint_xy.at (indexOf (index1, index0)) = true;
        m_isH_joint_xyz.at (indexOf (index0, index1, index2)) = true;
        m_isH_joint_xyz.at (indexOf (index1, index2, index0)) = true;
        m_isH_joint_xyz.at (indexOf (index2, index0, index1)) = true;
        m_isH_joint_xyz.at (indexOf (index0, index2, index1)) = true;
        m_isH_joint_xyz.at (indexOf (index2, index1, index0)) = true;
        m_isH_joint_xyz.at (indexOf (index1, index0, index2)) = true;
    }


private:

    inline double log_2 () const { return log (2); }

    inline double xi (double p) const { return p == 0.0 ? 0.0 : p * log (p)/log_2(); }
    inline double xi_cross (double p, double q) const { return q == 0.0 ? 0.0 : p * log (q)/log_2(); }
    inline int indexOf (const DataIndex& index) const { return index.index; }
    inline int indexOf (const DataIndex& index0, const DataIndex& index1) 
    { 
        return index0.index * m_numFeatures + index1.index; 
    }
    
    inline int indexOf (const DataIndex& index0, const DataIndex& index1, const DataIndex& index2) 
    { 
        return (index0.index * m_numFeatures + index1.index)*m_numFeatures + index2.index; 
    }

    const std::vector<std::vector<int> >& m_features; // last "feature" is the output
    const std::vector<double>& m_weights;
    int m_numFeatures; // includes the output 

    std::vector<DataIndex> m_dataIndices;

    std::vector<double> m_Hx;
    std::vector<double> m_H_joint_xy;
    std::vector<double> m_H_joint_xyz;

    std::vector<char> m_isHx;
    std::vector<char> m_isH_joint_xy;
    std::vector<char> m_isH_joint_xyz;
};


void printCmimFeatures (const std::vector<int>& selectedFeatures)
{
    std::cout << "CMIM" << std::endl;
    for (std::vector<int>::const_iterator it = selectedFeatures.begin (), itEnd = selectedFeatures.end (); it != itEnd; ++it)
    {
        std::cout << "feature " << (*it) << std::endl;
    }
    std::cout << std::endl;
}

void printComboFeature (MutualInformation& mutualInformation, const std::vector<std::vector<int> >& data, const CombinationFeature& combo)
{
    std::cout << "   ___ combination feature ___" << std::endl;

    std::vector<int>::const_iterator itFeature0Category = data.at (combo.index0).begin ();
    std::vector<int>::const_iterator itFeature0CategoryEnd = data.at (combo.index0).end ();
    std::vector<int>::const_iterator itFeature1Category = data.at (combo.index1).begin ();
    std::vector<int>::const_iterator itOutput = data.back ().begin ();
    for (; itFeature0Category != itFeature0CategoryEnd; ++itFeature0Category, ++itFeature1Category, ++itOutput)
    {
        int feature0Category = (*itFeature0Category);
        int feature1Category = (*itFeature1Category);
        int outputCategory   = (*itOutput);
        int comboCategory = combo.getComboOutput (feature0Category, feature1Category);
        std::cout << std::setw (4) << feature0Category
                  << std::setw (4) << feature1Category
                  << " => " 
                  << std::setw (4) << comboCategory
                  << " | " 
                  << std::setw (4) << outputCategory
                  << std::endl;
            
    }
    std::cout << std::endl;
}



bool test (const std::vector<std::vector<int> >& data, const std::vector<double>& weights)
{
    MutualInformation mutualInformation (data, weights);
    std::cout << std::endl;
    std::cout << " === Data == " << std::endl;
    for (int i = 0, iEnd = data.size (); i < iEnd; ++i)
    {
        std::cout << std::setw (3) << "F" << i;
    }
    std::cout << std::endl;
    for (int i = 0, iEnd = data.at(0).size (); i < iEnd; ++i)
    {
        for (std::vector<std::vector<int> >::const_iterator it = data.begin (), itEnd = data.end (); it != itEnd; ++it)
        {
            const int& value = (*it).at (i);
            std::cout << std::setw (4) << value;
        }
        std::cout << std::endl;
    }
    std::cout << " === Mutual information == " << std::endl;
    double h_0          = mutualInformation.H_x (0);
    double h_1          = mutualInformation.H_x (1);
    double h_2          = mutualInformation.H_x (2);
    double h_joint_01   = mutualInformation.H_joint_xy (0,1);
    double h_joint_02   = mutualInformation.H_joint_xy (0,2);
    double h_joint_12   = mutualInformation.H_joint_xy (1,2);
    double i_joint_01   = mutualInformation.I_joint_xy (0,1);
    double i_joint_02   = mutualInformation.I_joint_xy (0,2);
    double i_joint_12   = mutualInformation.I_joint_xy (1,2);
    double h_0_given_2  = mutualInformation.H_x_given_y (0,2);
    double h_2_given_0  = mutualInformation.H_x_given_y (2,0);
    double h_2_given_1  = mutualInformation.H_x_given_y (2,1);
    double h_1_given_2  = mutualInformation.H_x_given_y (1,2);
    double h_0_given_1  = mutualInformation.H_x_given_y (0,1);
    double h_1_given_0  = mutualInformation.H_x_given_y (1,0);
    double i_01_given_2 = mutualInformation.I_xy_given_z (0,1,2);
    double i_10_given_2 = mutualInformation.I_xy_given_z (1,0,2);
    double i_20_given_1 = mutualInformation.I_xy_given_z (2,0,1);
    double i_21_given_0 = mutualInformation.I_xy_given_z (2,1,0);
    double h_2_given_01 = mutualInformation.H_x_given_yz (2,1,0);
    double i_joint_012  = mutualInformation.I_joint_xyz (0,1,2);
    double i_int_012    = mutualInformation.I_int_xyz (0,1,2);
    double i_in_01_out_2 = mutualInformation.I_in_xy_out_z (0,1,2);

    double h_0__2        = mutualInformation.H_x (0);
    assert (abs(h_0 - h_0__2) < 0.0001);

    std::cout << ">>>> H(0)     = " << h_0 << std::endl;
    std::cout << ">>>> H(1)     = " << h_1 << std::endl;
    std::cout << ">>>> H(2)     = " << h_2 << std::endl;
    std::cout << ".... H(0,1)   = " << h_joint_01 << std::endl;
    std::cout << ".... H(1,2)   = " << h_joint_12 << std::endl;
    std::cout << ".... H(0,2)   = " << h_joint_02 << std::endl;
    std::cout << "---- I(0;1)   = " << i_joint_01 << std::endl;
    std::cout << "---- I(0;2)   = " << i_joint_02 << std::endl;
    std::cout << "---- I(1;2)   = " << i_joint_12 << std::endl;
    std::cout << "++++ H(0|2)   = " << h_0_given_2 << std::endl;
    std::cout << "++++ H(2|0)   = " << h_2_given_0 << std::endl;
    std::cout << "++++ H(2|1)   = " << h_2_given_1 << std::endl;
    std::cout << "++++ H(1|2)   = " << h_1_given_2 << std::endl;
    std::cout << "++++ H(0|1)   = " << h_0_given_1 << std::endl;
    std::cout << "++++ H(1|0)   = " << h_1_given_0 << std::endl;
    std::cout << ">>>> I(0;1|2) = " << i_01_given_2 << std::endl;
    std::cout << ">>>> I(1;0|2) = " << i_10_given_2 << std::endl;
    std::cout << ">>>> I(2;0|1) = " << i_20_given_1 << std::endl;
    std::cout << ">>>> I(2;1|0) = " << i_21_given_0 << std::endl;
    std::cout << "<<<< H(2|1,0) = " << h_2_given_01 << std::endl;
    std::cout << "---- I(0;1;2) = " << i_joint_012 << std::endl;
    std::cout << "---- Iint(0;1;2) = " << i_int_012 << std::endl;
    std::cout << "---- I(0,1;2) = " << i_in_01_out_2 << "     == I(2;0,1)" << std::endl;


//    bool decide = (h_2_given_01 < h_2_given_0 && h_2_given_01 < h_2_given_1 && h_0 > 0 && h_1 > 0);
    bool decide = (i_in_01_out_2 > i_joint_02 && i_in_01_out_2 > i_joint_12);

    std::cout << "====> " << decide  << std::endl;
    std::vector<CombinationFeature> combos = mutualInformation.combinationFeatures ();
    std::cout << "COMBOS" << std::endl;
    for (std::vector<CombinationFeature>::iterator it = combos.begin (), itEnd = combos.end (); it != itEnd; ++it)
    {
        CombinationFeature& combo = (*it);
        std::cout << "combo " << (combo.index0) <<  " with " << (combo.index1) << "   in01_0ut2=" << (combo.in01_out2) << "  joint02=" << (combo.joint02)  << "  joint12=" << (combo.joint12) << std::endl;
        mutualInformation.computeCombinationFeatureMapping (combo);
        printComboFeature (mutualInformation, data, combo);
    }
    std::cout << "---" << std::endl;
    printCmimFeatures (mutualInformation.cmim ());
    return decide;
}



int main ()
{

    feenableexcept (FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);


    int correct = 0;
    int wrong = 0;
    if (false)
    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,0,1,0,1}); 
        data.push_back ({1,0,1,0,1}); 
        data.push_back ({1,0,1,0,1}); 
        weights.assign (data.at(0).size (), 1.0);

        MutualInformation mutualInformation (data, weights);
        std::cout << " === Mutual information == " << std::endl;
        std::cout << ">>>> H(0)     = " << mutualInformation.H_x (0) << std::endl;
        std::cout << ">>>> H(1)     = " << mutualInformation.H_x (1) << std::endl;
        std::cout << ">>>> H(0,1)   = " << mutualInformation.H_joint_xy (0,1) << std::endl;
        std::cout << ">>>> I(0,1)   = " << mutualInformation.I_joint_xy (0,1) << std::endl;
        std::cout << ">>>> H(0|1)   = " << mutualInformation.H_x_given_y (0,1) << std::endl;
        std::cout << ">>>> H(0,1,2) = " << mutualInformation.H_joint_xyz (0,1,2) << std::endl;
        std::cout << ">>>> I(0,1,2) = " << mutualInformation.I_joint_xyz (0,1,2) << std::endl;
        std::cout << ">>>> I(0,1|2) = " << mutualInformation.I_xy_given_z (0,1,2) << std::endl;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,0,1,0,1,0,1,0}); 
        data.push_back ({1,1,0,0,1,1,0,0}); 
        data.push_back ({1,0,1,0,1,0,1,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++wrong : ++correct;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,0,1,0,1,0,1,0}); 
        data.push_back ({1,1,0,0,1,1,0,0}); 
        data.push_back ({1,0,0,1,1,0,0,1}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++correct : ++wrong;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,0,1,0,1,0,1,0}); 
        data.push_back ({1,1,0,0,1,1,0,0}); 
        data.push_back ({0,1,1,0,0,1,1,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++correct : ++wrong;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,1,1,1,0,0,0,0}); 
        data.push_back ({0,1,0,1,1,0,0,0}); 
        data.push_back ({1,0,1,0,1,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++correct : ++wrong;
    }


    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,1,1,1,0,0,0,0}); 
        data.push_back ({0,0,0,0,1,0,0,0}); 
        data.push_back ({1,1,1,1,1,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++correct : ++wrong;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,1,1,1,0,0,0,0}); 
        data.push_back ({0,0,0,0,1,0,0,0}); 
        data.push_back ({1,1,1,1,0,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++wrong : ++correct;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,1,1,1,0,0,0,0}); 
        data.push_back ({1,0,0,0,1,0,0,0}); 
        data.push_back ({1,1,1,1,0,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++wrong : ++correct;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,0,1,0,1,2,1,2}); 
//        data.push_back ({1,1,1,0,1,2,1,2}); 
        data.push_back ({2,1,2,1,0,1,0,1}); 
        data.push_back ({1,1,1,1,0,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++correct : ++wrong;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({0,0,0,0,0,0,0,0});  
        data.push_back ({1,1,1,1,0,0,0,0}); 
        data.push_back ({0,0,0,1,1,1,0,0}); 
        data.push_back ({1,0,0,0,1,0,0,0}); 
        data.push_back ({1,1,3,1,0,0,0,0}); 
        data.push_back ({1,2,1,4,0,0,0,0}); 
        data.push_back ({1,2,3,4,0,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++wrong : ++correct;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        // data.push_back ({1,1,3,1,0,0,0,0}); 
        // data.push_back ({1,2,1,4,0,0,0,0}); 
        data.push_back ({1,1,1,1,0,0,0,0}); 
        data.push_back ({1,0,1,0,0,0,0,0}); 
        data.push_back ({1,2,1,2,0,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++correct : ++wrong;
    }



    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({1,1,1,1,1,1,0,0,0,0,0}); 
        data.push_back ({0,0,0,0,0,0,1,0,0,0,0}); 
        data.push_back ({1,1,1,1,1,1,1,0,0,0,0}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++correct : ++wrong;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({0,1,2,3,4,5,6,7,8,9,10}); 
        data.push_back ({0,1,0,0,1,2,0,1,0,0,1}); 
//        data.push_back ({0,1,2,0,1,2,0,1,2,0,1}); 
        data.push_back ({0,1,0,0,1,2,0,1,0,0,1}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++wrong : ++correct;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back ({0,1,2,3,4,5,6,7,8,9,10}); 
        data.push_back ({0,1,1,0,1,2,0,1,0,0,1}); 
        data.push_back ({0,1,0,0,1,2,0,1,0,0,1}); 
        weights.assign (data.at(0).size (), 1.0);
        test (data, weights) ? ++wrong : ++correct;
    }


    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back    ({0,0,0,0,1,1,1,2,2,2,2}); 
        data.push_back    ({0,1,2,0,1,2,0,1,2,0,1}); 
        data.push_back    ({0,1,1,0,2,0,1,0,1,2,0}); 
//        weights =          {1,1,-0.1,1,1,1,1,1,1,1,1};
        weights =          {1,1,0.1,1,1,1,1,1,1,1,1};
        test (data, weights) ? ++wrong : ++correct;
    }

    {
        std::vector<std::vector<int> > data;
        std::vector<double> weights;
        data.push_back    ({0,0,0,0,0,0,0,0,0,0,0}); 
        data.push_back    ({1,2,3,4,5,6,7,8,9,10,11}); 
        data.push_back    ({0,1,1,0,2,0,1,0,1,2,0}); 
        weights =          {1,1,1,1,1,1,1,1,1,1,1};
        test (data, weights) ? ++wrong : ++correct;
    }



    std::cout << "result === " << std::endl;
    std::cout << "correct = " << correct << std::endl;
    std::cout << "wrong   = " << wrong << std::endl;
}


