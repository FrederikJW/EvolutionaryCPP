#include <tbb/tbb.h>
#include <emmintrin.h>
#include <immintrin.h> 
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "CPPSolutionBase.h"

// does not work correctly
void parallelAddSse(std::vector<int>& a, const std::vector<int>& b) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, a.size(), 4),
        [&a, &b](const tbb::blocked_range<size_t>& r) {
            
            for (size_t i = 0; i < r.size() * 4; i += 4) {
                __m128i a_values = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&a[i]));
                __m128i b_values = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&b[i]));
                __m128i result = _mm_add_epi32(a_values, b_values);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(&a[i]), result);
            }
        });
}

void parallelAddSse2(std::vector<int>& a, const std::vector<int>& b) {
    size_t size = a.size();

    // Ensure that both vectors have the same size
    if (size != b.size()) {
        throw std::invalid_argument("Vectors must have the same size");
    }

    for (size_t i = 0; i < size; i += 4) {
        // Load 4 integers from each vector into SSE registers
        __m128i va = _mm_loadu_si128((__m128i*) & a[i]);
        __m128i vb = _mm_loadu_si128((__m128i*) & b[i]);

        // Perform the addition
        __m128i vresult = _mm_add_epi32(va, vb);

        // Store the result back to vector a
        _mm_storeu_si128((__m128i*) & a[i], vresult);
    }
}

void shuffle(std::vector<int>& list, std::default_random_engine& iGenerator) {
    int n = list.size(); // The number of items left to shuffle (loop invariant).
    while (n > 1) {
        std::uniform_int_distribution<int> distribution(0, n - 1);
        int k = distribution(iGenerator); // 0 <= k < n.
        n--; // n is now the last pertinent index;
        int temp = list[n]; // swap array[n] with array[k] (does nothing if k == n).
        list[n] = list[k];
        list[k] = temp;
    }
}


void CPPSolutionBase::InitRestricted(int Size)
{
    mRestricted.clear();

    for (int i = 0; i < mInstance->getNumberOfNodes(); i++)
    {
        mRestricted.push_back(i);
    }

    shuffle(mRestricted, mGenerator);

    mRestricted.erase(mRestricted.begin() + Size - 1, mRestricted.end());
}

void CPPSolutionBase::Clear() {}

void CPPSolutionBase::InitChange() {}

bool CPPSolutionBase::CheckSolutionValid()
{
    return CheckSolutionValid(*mInstance);
}

void CPPSolutionBase::Load(const std::string& FileName)
{
    std::ifstream file(FileName);
    std::string line;
    std::vector<std::string> Lines;
    while (std::getline(file, line)) {
        Lines.push_back(line);
    }
    file.close();

    std::vector<std::vector<int>> tCliques(mInstance->getNumberOfNodes());
    std::vector<int> iNodeCliques(mInstance->getNumberOfNodes());

    for (int i = 0; i < mInstance->getNumberOfNodes(); i++)
    {
        std::stringstream ss(Lines[i]);
        std::string word;
        std::vector<std::string> words;
        while (std::getline(ss, word, ' ')) {
            words.push_back(word);
        }
        tCliques[std::stoi(words[1])].push_back(i);
    }

    mCliques.clear();
    for (const auto& l : tCliques)
    {
        if (!l.empty()) {
            mCliques.push_back(l);
        }
    }

    int result = CalculateObjective();
}

int CPPSolutionBase::CalculateObjectiveForClique(const std::vector<int>& Clique)
{
    int result = 0;
    for (int i : Clique)
    {
        for (int j : Clique)
        {
            if (i > j)
                result += mInstance->getWeights()[i][j];
        }
    }
    return result;
}

void CPPSolutionBase::FixCliques() {}

bool CPPSolutionBase::InSameClique(int nodeA, int nodeB)
{
    return mNodeClique[nodeA] == mNodeClique[nodeB];
}

int CPPSolutionBase::CalculateObjective()
{
    int result = 0;

    for (const auto& Clique : mCliques)
    {
        result += CalculateObjectiveForClique(Clique);
    }

    mObjective = result;
    return result;
}

void CPPSolutionBase::AddCandidate(CPPCandidate& A) {};

CPPSolutionBase::CPPSolutionBase()
{
    mCliques = std::vector<std::vector<int>>();
}

CPPSolutionBase::CPPSolutionBase(const CPPSolutionBase& iSolution)
{
    mObjective = iSolution.mObjective;
    mCliques.clear();
    for (const auto& l : iSolution.mCliques)
    {
        mCliques.push_back(l);
    }
    mNodeClique.resize(iSolution.mNodeClique.size());
    std::copy(iSolution.mNodeClique.begin(), iSolution.mNodeClique.end(), mNodeClique.begin());
}

CPPSolutionBase::CPPSolutionBase(int iObjective, std::vector<std::vector<int>> iPartitions)
{
    mObjective = iObjective;
    mCliques = std::move(iPartitions);
}

CPPSolutionBase::CPPSolutionBase(int* pvertex, int numVertices, int objective, CPPInstance* nInstance)
{   
    // do not delete mInstance if solution object is deleted
    mInstance = nInstance;
    std::unordered_map<int, std::vector<int>> groupMap;
    mNodeClique.resize(numVertices);

    for (int i = 0; i < numVertices; ++i) {
        groupMap[pvertex[i]].push_back(i);
        //mNodeClique[i] = pvertex[i];
    }

    std::vector<std::vector<int>> groups(groupMap.size());

    int groupIndex = 0;
    for (auto& pair : groupMap) {
        for (int j : pair.second) {
            mNodeClique[j] = groupIndex;
        }
        groups[groupIndex] = std::move(pair.second);
        groupIndex++;
    }

    mCliques = std::move(groups);
    mObjective = objective;
}

bool CPPSolutionBase::CheckSolutionValid(CPPInstance tInstance)
{
    int TotalNodes = 0;
    std::vector<int> AllNodes;

    for (const auto& Cli : mCliques)
    {
        TotalNodes += Cli.size();
        AllNodes.insert(AllNodes.end(), Cli.begin(), Cli.end());
    }

    std::sort(AllNodes.begin(), AllNodes.end());
    for (size_t i = 0; i < AllNodes.size() - 1; i++)
    {
        if (AllNodes[i] == AllNodes[i + 1])
            return false;
    }
    if (TotalNodes != tInstance.getNumberOfNodes())
        return false;

    for (int i = 0; i < tInstance.getNumberOfNodes(); i++)
    {
        if (std::find(mCliques[mNodeClique[i]].begin(), mCliques[mNodeClique[i]].end(), i) == mCliques[mNodeClique[i]].end())
            return false;
    }

    return true;
}

std::vector<int> CPPSolutionBase::CliqueForNode(int iNode)
{
    return mCliques[mNodeClique[iNode]];
}

bool CPPSolutionBase::IsSame(int iObjective, const std::vector<std::vector<int>>& iPartitions)
{
    if (iObjective != mObjective)
        return false;
    if (iPartitions.size() != mCliques.size())
        return false;

    int FirstClique;
    for (const auto& l : iPartitions)
    {
        if (l.empty())
            continue;

        FirstClique = mNodeClique[l[0]];
        for (int n : l)
        {
            if (FirstClique != mNodeClique[n])
                return false;
        }
    }

    return true;
}

int CPPSolutionBase::CalculateAllConnections(int node, int clique)
{
    int result = 0;

    for (int tn : mCliques[clique])
    {
        result += mInstance->getWeights()[tn][node];
    }

    return result;
}


void CPPSolutionBase::UpdateAllConnections(int nNode, int nClique)
{
    if (nClique >= static_cast<int>(mAllConnections.size()))
    {
        std::vector<std::vector<int>> nAllConnections(mAllConnections.size() + 1);
        nAllConnections[nClique].resize(mNodeClique.size(), 0);

        for (size_t i = 0; i < mAllConnections.size(); i++)
        {
            nAllConnections[i] = mAllConnections[i];
        }

        mAllConnections = nAllConnections;
    }

    std::vector<int>& newAllConnectedNode = mAllConnections[nClique];
    std::vector<int>& oldAllConnectedNode = mAllConnections[mNodeClique[nNode]];

    const std::vector<int>& NodeWeights = mInstance->getWeights()[nNode];
    const std::vector<int>& NodeNegativeWeights = mInstance->getNegativeWeights()[nNode];

    parallelAddSse2(newAllConnectedNode, NodeWeights);
    parallelAddSse2(oldAllConnectedNode, NodeNegativeWeights);
}

void CPPSolutionBase::UpdateAllConnectionsRestricted(int nNodeIndex, int nClique)
{
    int nNode = mRestricted[nNodeIndex];

    if (nClique >= static_cast<int>(mAllConnections.size()))
    {
        std::vector<std::vector<int>> nAllConnections(mAllConnections.size() + 1);
        nAllConnections[nClique].resize(mNodeClique.size(), 0);

        for (size_t i = 0; i < mAllConnections.size(); i++)
        {
            nAllConnections[i] = mAllConnections[i];
        }

        mAllConnections = nAllConnections;
    }

    std::vector<int>& newAllConnectedNode = mAllConnections[nClique];
    std::vector<int>& oldAllConnectedNode = mAllConnections[mNodeClique[nNode]];

    const std::vector<int>& NodeWeights = mInstance->getWeights()[nNode];
    int Size = mInstance->getNumberOfNodes();

    for (int tn : mRestricted)
    {
        newAllConnectedNode[tn] += NodeWeights[tn];
        oldAllConnectedNode[tn] -= NodeWeights[tn];
    }
}

void CPPSolutionBase::InitAllConnections()
{
    mAllConnections.resize(mCliques.size());

    for (size_t c = 0; c < mCliques.size(); c++)
    {
        mAllConnections[c].resize(mNodeClique.size());

        for (int tn = 0; tn < mInstance->getNumberOfNodes(); tn++)
        {
            mAllConnections[c][tn] = CalculateAllConnections(tn, c);
        }
    }

    mCliqueSizes.clear();
    for (const auto& cClique : mCliques)
    {
        mCliqueSizes.push_back(cClique.size());
    }
}

void CPPSolutionBase::CreateRelocations(int n1, int n2, int n3, int c, std::vector<std::array<int, 2>>& BestRelocations)
{
    BestRelocations.clear();

    BestRelocations.push_back({ n1, c });
    BestRelocations.push_back({ n2, c });
    BestRelocations.push_back({ n3, c });
}

void CPPSolutionBase::CreateRelocations(int n1, int n2, int c, std::vector<std::array<int, 2>>& BestRelocations)
{
    BestRelocations.clear();

    BestRelocations.push_back({ n1, c });
    BestRelocations.push_back({ n2, c });
}

void CPPSolutionBase::SimulatedAnnealingSelectTrio(int n1, int n2, int n3, int& BestChange, std::vector<std::array<int, 2>>& BestRelocations, std::default_random_engine& iGenerator)
{
    int n1Clique, n2Clique, n3Clique;
    int cChange;

    if ((mNodeClique[n1] == mNodeClique[n2]) && (mNodeClique[n2] == mNodeClique[n3]))
    {
        n1Clique = mNodeClique[n1];
        for (int c = 0; c < static_cast<int>(mCliques.size()); c++)
        {
            if (c != n1Clique)
            {
                cChange = mAllConnections[c][n1] + mAllConnections[c][n2] + mAllConnections[c][n3]
                    + 2 * mInstance->getWeights()[n1][n2] + 2 * mInstance->getWeights()[n1][n3] + 2 * mInstance->getWeights()[n2][n3]
                    - mAllConnections[n1Clique][n1] - mAllConnections[n1Clique][n2] - mAllConnections[n1Clique][n3];

                if (cChange > BestChange)
                {
                    BestChange = cChange;
                    CreateRelocations(n1, n2, n3, c, BestRelocations);
                }
            }
        }
        return;
    }

    if ((mNodeClique[n1] != mNodeClique[n2]) && (mNodeClique[n2] != mNodeClique[n3]) && (mNodeClique[n1] != mNodeClique[n3]))
    {
        n1Clique = mNodeClique[n1];
        n2Clique = mNodeClique[n2];
        n3Clique = mNodeClique[n3];

        for (int c = 0; c < static_cast<int>(mCliques.size()); c++)
        {
            if ((c != n1Clique) && (c != n2Clique) && (c != n3Clique))
            {
                cChange = mAllConnections[c][n1] + mAllConnections[c][n2] + mAllConnections[c][n3]
                    + mInstance->getWeights()[n1][n2] + mInstance->getWeights()[n1][n3] + mInstance->getWeights()[n2][n3]
                    - mAllConnections[n1Clique][n1] - mAllConnections[n2Clique][n2] - mAllConnections[n3Clique][n3];

                if (cChange > BestChange)
                {
                    BestChange = cChange;
                    CreateRelocations(n1, n2, n3, c, BestRelocations);
                }
            }
        }
        return;
    }

    int tN1, tN2, tN3;

    if (mNodeClique[n1] == mNodeClique[n2])
    {
        tN1 = n1;
        tN2 = n2;
        tN3 = n3;
    }
    else if (mNodeClique[n1] == mNodeClique[n3])
    {
        tN1 = n1;
        tN2 = n3;
        tN3 = n2;
    }
    else
    {
        tN1 = n2;
        tN2 = n3;
        tN3 = n1;
    }

    n1Clique = mNodeClique[tN1];
    n3Clique = mNodeClique[tN3];

    for (int c = 0; c < static_cast<int>(mCliques.size()); c++)
    {
        if ((c != n1Clique) && (c != n3Clique))
        {
            cChange = mAllConnections[c][tN1] + mAllConnections[c][tN2] + mAllConnections[c][tN3]
                + 2 * mInstance->getWeights()[tN1][tN2] + mInstance->getWeights()[tN1][tN3] + mInstance->getWeights()[tN2][tN3]
                - mAllConnections[n1Clique][tN1] - mAllConnections[n1Clique][tN2] - mAllConnections[n3Clique][tN3];

            if (cChange > BestChange)
            {
                BestChange = cChange;
                CreateRelocations(n1, n2, n3, c, BestRelocations);
            }
        }
    }
}

void CPPSolutionBase::SASelectDual(SARelocation& Relocation)
{
    std::vector<int> CliqueConnections;

    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];
    int n0RemoveChange;
    int n1RemoveChange;
    int RemoveChange;

    int bWeight;
    int cChange0;
    int cChange1;
    int cChange;
    int swapChange;
    int Size = mCliqueSizes.size();

    n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    bWeight = mInstance->getWeights()[Relocation.mN0][Relocation.mN1];

    if (mCliqueSizes[n0Clique] > Relocation.mChange)
    {
        Relocation.mChange = n0RemoveChange;
        Relocation.mC0 = Size;
        Relocation.mMoveType = SAMoveType::N0;
    }

    for (int c = 0; c < Size; c++)
    {
        CliqueConnections = mAllConnections[c];

        cChange0 = n0RemoveChange + CliqueConnections[Relocation.mN0];
        if (n0Clique != c)
        {
            if (cChange0 > Relocation.mChange)
            {
                Relocation.mChange = cChange0;
                Relocation.mC0 = c;
                Relocation.mMoveType = SAMoveType::N0;
            }
        }
        cChange1 = n1RemoveChange + CliqueConnections[Relocation.mN1];

        if ((n1Clique != c) && (n0Clique != c))
        {
            if (n1Clique == n0Clique)
            {
                cChange = cChange1 + cChange0 + 2 * bWeight;
            }
            else
            {
                cChange = cChange1 + cChange0 + bWeight;
            }

            if (cChange > Relocation.mChange)
            {
                Relocation.mChange = cChange;
                Relocation.mC0 = c;
                Relocation.mC1 = c;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }

        if (n0Clique != n1Clique)
        {
            if (c != n0Clique)
            {
                if (c != n1Clique)
                {
                    cChange = cChange0 + n1RemoveChange + mAllConnections[n0Clique][Relocation.mN1] - bWeight;
                }
                else
                {
                    cChange = cChange0 + n1RemoveChange + mAllConnections[n0Clique][Relocation.mN1] - 2 * bWeight;
                }

                if (cChange > Relocation.mChange)
                {
                    Relocation.mChange = cChange;
                    Relocation.mC0 = c;
                    Relocation.mC1 = n0Clique;
                    Relocation.mMoveType = SAMoveType::Slide;
                }
            }
        }
    }
}

void CPPSolutionBase::SASelectDualPrev(SARelocation& Relocation)
{
    std::vector<int> CliqueConnections;

    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];
    int n0RemoveChange;
    int n1RemoveChange;
    int RemoveChange;
    int bWeight;
    int cChange0;
    int cChange1;
    int cChange;
    int swapChange;
    int Size = mCliqueSizes.size();

    n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    bWeight = mInstance->getWeights()[Relocation.mN0][Relocation.mN1];

    if (n0RemoveChange < n1RemoveChange)
    {
        if (mCliqueSizes[n1Clique] > 1)
        {
            Relocation.mChange = n1RemoveChange;
            Relocation.mC1 = Size;
            Relocation.mMoveType = SAMoveType::N1;
        }
    }
    else
    {
        if (mCliqueSizes[n0Clique] > 1)
        {
            Relocation.mChange = n0RemoveChange;
            Relocation.mC0 = Size;
            Relocation.mMoveType = SAMoveType::N0;
        }
    }

    for (int c = 0; c < Size; c++)
    {
        CliqueConnections = mAllConnections[c];

        cChange0 = n0RemoveChange + CliqueConnections[Relocation.mN0];
        if (n0Clique != c)
        {
            if (cChange0 > Relocation.mChange)
            {
                Relocation.mChange = cChange0;
                Relocation.mC0 = c;
                Relocation.mMoveType = SAMoveType::N0;
            }
        }
        cChange1 = n1RemoveChange + CliqueConnections[Relocation.mN1];

        if ((n1Clique != c) && (n0Clique != c))
        {
            if (n1Clique == n0Clique)
            {
                cChange = cChange1 + cChange0 + 2 * bWeight;
            }
            else
            {
                cChange = cChange1 + cChange0 + bWeight;
            }

            if (cChange > Relocation.mChange)
            {
                Relocation.mChange = cChange;
                Relocation.mC0 = c;
                Relocation.mC1 = c;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }
    }
}

void CPPSolutionBase::SASelectDualExt(SARelocation& Relocation)
{
    std::vector<int> CliqueConnections;

    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];
    int n0RemoveChange;
    int n1RemoveChange;
    int RemoveChange;
    int bWeight;
    int cChange0;
    int cChange1;
    int cChange;
    int swapChange;
    int Size = mCliqueSizes.size();

    n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    bWeight = mInstance->getWeights()[Relocation.mN0][Relocation.mN1];

    if (n0RemoveChange < n1RemoveChange)
    {
        if (mCliqueSizes[n1Clique] > 1)
        {
            Relocation.mChange = n1RemoveChange;
            Relocation.mC1 = Size;
            Relocation.mMoveType = SAMoveType::N1;
        }
    }
    else
    {
        if (mCliqueSizes[n0Clique] > 1)
        {
            Relocation.mChange = n0RemoveChange;
            Relocation.mC0 = Size;
            Relocation.mMoveType = SAMoveType::N0;
        }
    }

    for (int c = 0; c < Size; c++)
    {
        CliqueConnections = mAllConnections[c];

        cChange0 = n0RemoveChange + CliqueConnections[Relocation.mN0];
        if (n0Clique != c)
        {
            if (cChange0 > Relocation.mChange)
            {
                Relocation.mChange = cChange0;
                Relocation.mC0 = c;
                Relocation.mMoveType = SAMoveType::N0;
            }
        }
        cChange1 = n1RemoveChange + CliqueConnections[Relocation.mN1];

        if (n1Clique != c)
        {
            if (cChange1 > Relocation.mChange)
            {
                Relocation.mChange = cChange1;
                Relocation.mC1 = c;
                Relocation.mMoveType = SAMoveType::N1;
            }
        }

        if ((n1Clique != c) && (n0Clique != c))
        {
            if (n1Clique == n0Clique)
            {
                cChange = cChange1 + cChange0 + 2 * bWeight;
            }
            else
            {
                cChange = cChange1 + cChange0 + bWeight;
            }

            if (cChange > Relocation.mChange)
            {
                Relocation.mChange = cChange;
                Relocation.mC0 = c;
                Relocation.mC1 = c;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }

        if (n0Clique != n1Clique)
        {
            if (c != n0Clique)
            {
                if (c != n1Clique)
                {
                    cChange = cChange0 + n1RemoveChange + mAllConnections[n0Clique][Relocation.mN1] - bWeight;
                }
                else
                {
                    cChange = cChange0 + n1RemoveChange + mAllConnections[n0Clique][Relocation.mN1] - 2 * bWeight;
                }

                if (cChange > Relocation.mChange)
                {
                    Relocation.mChange = cChange;
                    Relocation.mC0 = c;
                    Relocation.mC1 = n0Clique;
                    Relocation.mMoveType = SAMoveType::Slide;
                }
            }

            if (c != n1Clique)
            {
                if (c != n0Clique)
                {
                    cChange = cChange1 + n0RemoveChange + mAllConnections[n1Clique][Relocation.mN0] - bWeight;
                }
                else
                {
                    cChange = cChange1 + n0RemoveChange + mAllConnections[n1Clique][Relocation.mN0] - 2 * bWeight;
                }

                if (cChange > Relocation.mChange)
                {
                    Relocation.mChange = cChange;
                    Relocation.mC0 = n1Clique;
                    Relocation.mC1 = c;
                    Relocation.mMoveType = SAMoveType::Slide;
                }
            }
        }
    }

    if (n1Clique != n0Clique)
    {
        swapChange = n0RemoveChange + n1RemoveChange + mAllConnections[n0Clique][Relocation.mN1] + mAllConnections[n1Clique][Relocation.mN0] - 2 * bWeight;
        if (swapChange > Relocation.mChange)
        {
            Relocation.mChange = swapChange;
            Relocation.mMoveType = SAMoveType::Swap;
        }
    }

    n0RemoveChange += n1RemoveChange;

    if (n1Clique == n0Clique)
    {
        if (Relocation.mChange < n0RemoveChange + 2 * bWeight)
        {
            Relocation.mChange = n0RemoveChange + 2 * bWeight;
            Relocation.mC0 = Size;
            Relocation.mC1 = Size;
            Relocation.mMoveType = SAMoveType::Both;
        }
    }
    else
    {
        if (Relocation.mChange < n0RemoveChange + bWeight)
        {
            Relocation.mChange = n0RemoveChange + bWeight;
            Relocation.mC0 = Size;
            Relocation.mC1 = Size;
            Relocation.mMoveType = SAMoveType::Both;
        }
    }
}

void CPPSolutionBase::SASelectSingle(SARelocation& Relocation)
{
    std::vector<int> CliqueConnections;

    int n0Clique = mNodeClique[Relocation.mN0];
    int n0RemoveChange;
    int cChange0;
    int Size = mCliqueSizes.size();

    n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];

    if (mCliqueSizes[n0Clique] > 1)
    {
        Relocation.mChange = n0RemoveChange;
        Relocation.mC0 = Size;
        Relocation.mMoveType = SAMoveType::N0;
    }

    for (int c = 0; c < Size; c++)
    {
        CliqueConnections = mAllConnections[c];

        cChange0 = n0RemoveChange + CliqueConnections[Relocation.mN0];
        if (n0Clique != c)
        {
            if (cChange0 > Relocation.mChange)
            {
                Relocation.mChange = cChange0;
                Relocation.mC0 = c;
                Relocation.mMoveType = SAMoveType::N0;
            }
        }
    }
}

void CPPSolutionBase::SASelectDual(SARelocation& Relocation, std::default_random_engine& iGenerator)
{
    int n0, n1;

    std::uniform_int_distribution<int> distribution(0, mInstance->getNumberOfNodes() - 1);
    n0 = distribution(iGenerator);

    Relocation.mN1 = Relocation.mN0;
    Relocation.mN0 = n0;

    if (Relocation.mN0 != Relocation.mN1)
        SASelectDual(Relocation);
    else
        SASelectSingle(Relocation);
}

void CPPSolutionBase::SASelectSingle(SARelocation& Relocation, std::default_random_engine& iGenerator)
{
    int n0, n1;

    std::uniform_int_distribution<int> distribution(0, mInstance->getNumberOfNodes() - 1);
    n0 = distribution(iGenerator);

    Relocation.mN0 = n0;

    SASelectSingle(Relocation);
}

void CPPSolutionBase::SASelect(SARelocation& Relocations, std::default_random_engine& iGenerator)
{
    switch (mSASelectType)
    {
    case SASelectType::Single:
        SASelectSingle(Relocations, iGenerator);
        break;
    case SASelectType::Dual:
        SASelectDual(Relocations, iGenerator);
        break;
    default:
        SASelectDual(Relocations, iGenerator);
        break;
    }
}

void CPPSolutionBase::ApplyRelocation(SARelocation Relocation)
{
    switch (Relocation.mMoveType)
    {
    case SAMoveType::N0:
        UpdateAllConnections(Relocation.mN0, Relocation.mC0);
        MoveNodeSA(Relocation.mN0, Relocation.mC0);
        break;
    case SAMoveType::N1:
        UpdateAllConnections(Relocation.mN1, Relocation.mC1);
        MoveNodeSA(Relocation.mN1, Relocation.mC1);
        break;
    case SAMoveType::Both:
        UpdateAllConnections(Relocation.mN0, Relocation.mC0);
        MoveNodeSA(Relocation.mN0, Relocation.mC0);
        UpdateAllConnections(Relocation.mN1, Relocation.mC1);
        MoveNodeSA(Relocation.mN1, Relocation.mC1);
        break;
    case SAMoveType::Swap:
        int oldC0;
        oldC0 = mNodeClique[Relocation.mN0];
        UpdateAllConnections(Relocation.mN0, mNodeClique[Relocation.mN1]);
        MoveNodeSA(Relocation.mN0, mNodeClique[Relocation.mN1]);
        UpdateAllConnections(Relocation.mN1, oldC0);
        MoveNodeSA(Relocation.mN1, oldC0);
        break;
    case SAMoveType::Slide:
        UpdateAllConnections(Relocation.mN0, Relocation.mC0);
        MoveNodeSA(Relocation.mN0, Relocation.mC0);
        UpdateAllConnections(Relocation.mN1, Relocation.mC1);
        MoveNodeSA(Relocation.mN1, Relocation.mC1);
        break;
    }

    while (RemoveEmptyCliqueSA(true, true));
}

bool CPPSolutionBase::SimulatedAnnealing(std::default_random_engine& iGenerator, SAParameters& iSAParameters, double& AcceptRelative)
{   
    // printf("enter SA");
    int NeiborhoodSize = mInstance->getNumberOfNodes() * getNumberOfCliques();
    int n;
    double Prob;
    double T = 1;
    int BestSol = INT_MIN;
    int cSol = INT_MIN;
    std::vector<int> tNodeClique(mInstance->getNumberOfNodes());
    int StartObjective = CalculateObjective();
    int cSolObjective;
    int Accept;
    int AcceptTotal;
    int Stag = 0;
    int counter = 0;
    std::vector<int> NodesChange(mInstance->getNumberOfNodes());
    SARelocation cRelocation;
    int waste = 0;

    InitAllConnections();
    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());

    T = iSAParameters.mInitTemperature;

    int counterCool = 0;
    cSolObjective = StartObjective;
    AcceptTotal = 0;
    int sumNodeChange;
    while (true)
    {   
        // gets caught in an endless loop here?
        counter++;
        Accept = 0;
        waste = 0;

        for (int i = 0; i < NeiborhoodSize * iSAParameters.mSizeRepeat; i++)
        {
            cRelocation.mChange = INT_MIN;
            SASelect(cRelocation, iGenerator);
            Prob = FastExp(cRelocation.mChange / T);
            if (cRelocation.mChange / T > 900 || cRelocation.mChange / T < -900)
                printf("  Prob=%.3f x=%.3f T=%.3f mchange=%d\n", Prob, cRelocation.mChange / T, T, cRelocation.mChange);
            if (Prob * 1000 > 1 + iGenerator() % 1000)
            {
                Accept++;
                ApplyRelocation(cRelocation);
                cSolObjective += cRelocation.mChange;
                if (cSol != cSolObjective)
                    cSol = cSolObjective;

                if (BestSol < cSolObjective)
                {
                    BestSol = cSolObjective;
                    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());
                }
            }
        }

        counterCool++;

        if (iSAParameters.mCooling == CPPCooling::Geometric)
        {
            T *= iSAParameters.mCoolingParam;
        }
        if (iSAParameters.mCooling == CPPCooling::LinearMultiplicative)
        {
            T = iSAParameters.mInitTemperature * 1 / (1 + iSAParameters.mCoolingParam * counterCool);
        }

        if (static_cast<double>(Accept) / (NeiborhoodSize * iSAParameters.mSizeRepeat) < iSAParameters.mMinAccept)
        {
            Stag++;
        }
        else
        {
            Stag = 0;
        }

        if (Stag >= 5)
            break;

        if (T < 0.0005)
            break;
        // printf("Stag=%d T=%.3f\n", Stag, T);
    }

    CreateFromNodeClique(tNodeClique);

    AcceptRelative = static_cast<double>(AcceptTotal) / (NeiborhoodSize * iSAParameters.mSizeRepeat * counter);
    // printf("exit SA");
    return true;
}

bool CPPSolutionBase::CalibrateSA(std::default_random_engine& iGenerator, SAParameters& iSAParameters, double& Accept)
{
    int MaxStep = mInstance->getNumberOfNodes() * getNumberOfCliques() * iSAParameters.mSizeRepeat;
    int n;
    double Prob;
    double T = 1;
    int BestSol = INT_MIN;
    int cSol = INT_MIN;
    std::vector<int> tNodeClique(mInstance->getNumberOfNodes());
    int StartObjective = CalculateObjective();
    int cSolObjective;
    int NoImprove = 0;
    Accept = 0;
    SARelocation Relocation;

    InitAllConnections();
    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());

    cSolObjective = StartObjective;
    for (int i = 0; i < MaxStep; i++)
    {
        NoImprove++;

        Relocation.mChange = INT_MIN;
        SASelect(Relocation, iGenerator);
        T = iSAParameters.mInitTemperature;

        Prob = std::exp(Relocation.mChange / T);

        if (Prob * 1000 > iGenerator() % 1000)
        {
            Accept++;

            ApplyRelocation(Relocation);

            cSolObjective += Relocation.mChange;
            if (cSol != cSolObjective)
                cSol = cSolObjective;

            if (BestSol < cSolObjective)
            {
                NoImprove = 0;
                BestSol = cSolObjective;
                std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());
            }
        }
    }

    CreateFromNodeClique(tNodeClique);

    Accept = Accept / MaxStep;
    return true;
}

double CPPSolutionBase::FastExp(double x)
{
    union {
        long long int i;
        double d;
    } tmp;

    tmp.i = static_cast<long long int>(1512775 * x + 1072632447);
    tmp.i <<= 32;
    return tmp.d;
}

int CPPSolutionBase::CalculateMoveChange(const std::vector<int>& iNodes, int iClique)
{
    int Remove = 0;
    int Add = 0;
    int counter = 0;

    for (size_t i = 0; i < iNodes.size(); ++i)
    {
        int n1 = iNodes[i];
        for (size_t j = i + 1; j < iNodes.size(); ++j)
        {
            int n2 = iNodes[j];
            if (InSameClique(n1, n2))
            {
                Remove += mInstance->getWeights()[n1][n2];
            }
            Add += mInstance->getWeights()[n1][n2];
        }
    }

    for (int n1 : iNodes)
    {
        for (int n2 : mCliques[mNodeClique[n1]])
        {
            if (std::find(iNodes.begin(), iNodes.end(), n2) == iNodes.end())
            {
                Remove += mInstance->getWeights()[n1][n2];
            }
        }
    }

    if (iClique < static_cast<int>(mCliques.size()))
    {
        for (int n1 : iNodes)
        {
            for (int n2 : mCliques[iClique])
            {
                if (std::find(iNodes.begin(), iNodes.end(), n2) == iNodes.end())
                {
                    Add += mInstance->getWeights()[n1][n2];
                }
            }
        }
    }

    return Add - Remove;
}

int CPPSolutionBase::CalculateMoveChange(BufferElement Set)
{
    int Remove = 0;
    int Add = 0;
    int counter = 0;

    for (size_t i = 0; i < Set.mRelocations.size(); ++i)
    {
        const auto& r1 = Set.mRelocations[i];
        for (size_t j = i + 1; j < Set.mRelocations.size(); ++j)
        {
            const auto& r2 = Set.mRelocations[j];
            if (InSameClique(r1[0], r2[0]))
            {
                Remove += mInstance->getWeights()[r1[0]][r2[0]];
            }

            if (r1[1] == r2[1])
            {
                Add += mInstance->getWeights()[r1[0]][r2[0]];
            }
        }
    }

    for (const auto& r1 : Set.mRelocations)
    {
        for (int n2 : mCliques[mNodeClique[r1[0]]])
        {
            if (!Set.Contains(n2))
            {
                Remove += mInstance->getWeights()[r1[0]][n2];
            }
        }
    }

    for (const auto& r1 : Set.mRelocations)
    {
        if (r1[1] < static_cast<int>(mCliques.size()))
        {
            for (int n : mCliques[r1[1]])
            {
                if (!Set.Contains(n))
                {
                    Add += mInstance->getWeights()[r1[0]][n];
                }
            }
        }
    }

    return Add - Remove;
}

int CPPSolutionBase::CalculateMoveChange(int iNode, int iClique, int iNodeRemoveChange)
{
    int result;
    if (mNodeClique[iNode] == iClique)
        return 0;

    if (iNodeRemoveChange != INT_MIN)
    {
        result = iNodeRemoveChange;
    }
    else
    {
        result = CalculateRemoveChange(iNode);
    }

    if (iClique >= static_cast<int>(mCliques.size()))
        return -result;

    result = CalculateAddChange(iNode, iClique) - result;

    return result;
}

bool CPPSolutionBase::CheckMove(int iNode, int iClique, int iNodeRemoveChange)
{
    return CalculateMoveChange(iNode, iClique, iNodeRemoveChange) > 0;
}

bool CPPSolutionBase::MoveNode(int iNode, int NewClique)
{
    auto& currentClique = mCliques[mNodeClique[iNode]];
    currentClique.erase(std::remove(currentClique.begin(), currentClique.end(), iNode), currentClique.end());

    mNodeClique[iNode] = NewClique;

    if (static_cast<int>(mCliques.size()) <= NewClique)
    {
        mCliques.resize(NewClique + 1);
    }

    if (mCliques[NewClique].empty())
    {
        mCliques[NewClique] = std::vector<int>();
    }

    mCliques[NewClique].push_back(iNode);

    return true;
}

bool CPPSolutionBase::MoveNodeSA(int iNode, int NewClique)
{
    mCliqueSizes[mNodeClique[iNode]]--;

    mNodeClique[iNode] = NewClique;

    if (static_cast<int>(mCliqueSizes.size()) <= NewClique)
    {
        mCliqueSizes.push_back(1);
    }
    else
    {
        mCliqueSizes[NewClique]++;
    }

    return true;
}

bool CPPSolutionBase::RemoveEmptyClique(bool bUpdateAllConnections, bool bUseCliqueSize)
{
    for (size_t i = 0; i < mCliques.size(); i++)
    {
        if (mCliques[i].empty())
        {
            mCliques.erase(mCliques.begin() + i);

            if (bUseCliqueSize)
            {
                mCliqueSizes.erase(mCliqueSizes.begin() + i);
            }

            for (size_t j = 0; j < mNodeClique.size(); j++)
            {
                if (mNodeClique[j] > static_cast<int>(i))
                {
                    mNodeClique[j]--;
                }
            }

            if (bUpdateAllConnections)
            {
                std::vector<std::vector<int>> nAllConnections(mAllConnections.size() - 1);

                size_t counter = 0;
                for (size_t ii = 0; ii < mAllConnections.size(); ii++)
                {
                    if (ii != i)
                    {
                        nAllConnections[counter++] = mAllConnections[ii];
                    }
                }

                mAllConnections = nAllConnections;
            }

            return true;
        }
    }

    return false;
}

bool CPPSolutionBase::RemoveEmptyCliqueSA(bool bUpdateAllConnections, bool bUseCliqueSize)
{
    for (size_t i = 0; i < mCliqueSizes.size(); i++)
    {
        if (mCliqueSizes[i] == 0)
        {
            if (bUseCliqueSize)
            {
                mCliqueSizes.erase(mCliqueSizes.begin() + i);
            }

            for (size_t j = 0; j < mNodeClique.size(); j++)
            {
                if (mNodeClique[j] > static_cast<int>(i))
                {
                    mNodeClique[j]--;
                }
            }

            if (bUpdateAllConnections)
            {
                std::vector<std::vector<int>> nAllConnections(mAllConnections.size() - 1);

                size_t counter = 0;
                for (size_t ii = 0; ii < mAllConnections.size(); ii++)
                {
                    if (ii != i)
                    {
                        nAllConnections[counter++] = mAllConnections[ii];
                    }
                }

                mAllConnections = nAllConnections;
            }

            return true;
        }
    }

    return false;
}

bool CPPSolutionBase::LocalSearch(std::vector<std::vector<int>> Nodes)
{
    std::vector<BufferElement> currentBuffer;
    int counter = 0;
    bool Result = false;

    InitAllConnections();
    while (true)
    {
        // if (!ImproveMoveExt(currentBuffer, iGenerator))
        if (!ImproveMove())
        {
            if (Nodes.empty())
                return Result;
            if (!ImproveMove(Nodes))
                return Result;
        }
        RemoveEmptyClique();
        Result = true;
        counter++;
    }
}

bool CPPSolutionBase::ImproveMove(std::vector<std::vector<int>>& Nodes)
{
    std::vector<int> shuffleCliques;
    int change;
    int c;

    for (int i = 0; i <= static_cast<int>(mCliques.size()); i++)
    {
        shuffleCliques.push_back(i);
    }

    shuffle(shuffleCliques, mGenerator);

    for (const auto& l : Nodes)
    {
        for (size_t tc = 0; tc < mCliques.size(); tc++)
        {
            c = shuffleCliques[tc];
            change = CalculateMoveChange(l, c);

            if (change > 0)
            {
                int Objective1 = CalculateObjective();
                for (int n : l)
                {
                    if (mNodeClique[n] != c)
                    {
                        MoveNode(n, c);
                    }
                }
                int Objective2 = CalculateObjective();
                return true;
            }
        }
    }

    return false;
}

bool CPPSolutionBase::ImproveMove()
{
    int cRemoveChange;
    int cChangeRelocate;
    std::vector<int> shuffleNodes;
    std::vector<int> shuffleCliques;

    for (int i = 0; i < mInstance->getNumberOfNodes(); i++)
    {
        shuffleNodes.push_back(i);
    }

    shuffle(shuffleNodes, mGenerator);

    for (size_t i = 0; i < mCliques.size(); i++)
    {
        shuffleCliques.push_back(i);
    }

    shuffle(shuffleCliques, mGenerator);

    int n;
    int c;

    for (int tn = 0; tn < mInstance->getNumberOfNodes(); tn++)
    {
        n = shuffleNodes[tn];
        cRemoveChange = mAllConnections[mNodeClique[n]][n];

        for (size_t tc = 0; tc < mCliques.size(); tc++)
        {
            c = shuffleCliques[tc];
            if (mNodeClique[n] != c)
            {
                cChangeRelocate = mAllConnections[c][n] - mAllConnections[mNodeClique[n]][n];

                if (cChangeRelocate > 0)
                {
                    UpdateAllConnections(n, c);
                    MoveNode(n, c);
                    RemoveEmptyClique(true);

                    return true;
                }
            }
        }

        if (cRemoveChange < 0)
        {
            UpdateAllConnections(n, mCliques.size());
            MoveNode(n, mCliques.size());
            RemoveEmptyClique(true);
            return true;
        }
    }

    return false;
}

void CPPSolutionBase::ExpandedBuffer(std::vector<BufferElement>& AllTest, int iNode, int iClique, int oClique)
{
    std::vector<BufferElement> newAllTest;
    BufferElement tBufferElement;

    for (const auto& CurrentSet : AllTest)
    {
        if (CurrentSet.CanAdd(iNode, iClique))
        {
            tBufferElement = BufferElement(CurrentSet);
            tBufferElement.Add(iNode, iClique, oClique);
            newAllTest.push_back(tBufferElement);
        }
    }

    for (const auto& cSet : newAllTest)
    {
        AllTest.push_back(cSet);
    }
}

int CPPSolutionBase::CalculateSwap(int A, const std::vector<int>& CliqueA, int B, const std::vector<int>& CliqueB)
{
    int result = 0;
    int NewA = 0;
    int NewB = 0;
    int OldA = 0;
    int OldB = 0;

    for (int n : CliqueA)
    {
        if (n != A)
        {
            OldA += mInstance->getWeights()[n][A];
            NewB += mInstance->getWeights()[n][B];
        }
    }

    for (int n : CliqueB)
    {
        if (n != B)
        {
            OldB += mInstance->getWeights()[n][B];
            NewA += mInstance->getWeights()[n][A];
        }
    }

    return NewA + NewB - OldA - OldB;
}

int CPPSolutionBase::CalculateRemoveChange(int iNode)
{
    int result = 0;

    for (int tNode : mCliques[mNodeClique[iNode]])
    {
        result += mInstance->getWeights()[iNode][tNode];
    }

    return result;
}

int CPPSolutionBase::CalculateAddChange(int iNode, int iClique)
{
    int result = mInstance->getWeights()[iNode][iNode];

    for (int tNode : mCliques[iClique])
    {
        result += mInstance->getWeights()[iNode][tNode];
    }

    return result;
}

void CPPSolutionBase::CreateFromNodeClique(const std::vector<int>& iNodeClique)
{
    mNodeClique.resize(iNodeClique.size());
    std::copy(iNodeClique.begin(), iNodeClique.end(), mNodeClique.begin());

    mCliques.clear();
    int iNumCliques = -1;

    for (int i = 0; i < static_cast<int>(mNodeClique.size()); i++)
    {
        if (iNumCliques < mNodeClique[i] + 1)
            iNumCliques = mNodeClique[i] + 1;
    }

    mCliques.resize(iNumCliques);

    for (int i = 0; i < static_cast<int>(mNodeClique.size()); i++)
    {
        mCliques[mNodeClique[i]].push_back(i);
    }
}

