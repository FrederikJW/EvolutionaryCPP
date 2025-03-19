// a rewritten version of CPPSolutionBase.cs into C++ https://github.com/rakabog/CPPConsole/blob/master/CPPConsole/CPPSolutionBase.cs
// with additional functionality for simulated annealing variations presented in this repository
#include <tbb/tbb.h>
#include <emmintrin.h>
#include <immintrin.h> 
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cmath>
#include <Dense>
#include "CPPSolutionBase.h"
#include "../RandomGenerator.h"
#include "../util/Util.h"
#include <algorithm>

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

// too slow
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

// too slow
void parallelAddSse3(std::vector<int>& a, const std::vector<int>& b) {
    Eigen::VectorXi vec_a = Eigen::Map<Eigen::VectorXi>(a.data(), a.size());
    Eigen::VectorXi vec_b = Eigen::Map<const Eigen::VectorXi>(b.data(), b.size());

    vec_a += vec_b;
}

// fastest, but can only be used py processors that support AVX-512. Use parallelAddSse5 if yours does not.
void parallelAddSse4(std::vector<int>& a, const std::vector<int>& b) {
    const size_t size = a.size();

    // Ensure both vectors are the same size
    if (b.size() != size) {
        throw std::invalid_argument("Vectors must be the same size");
    }

    size_t i = 0;

    // Process 16 integers at a time using AVX-512 intrinsics
    for (; i + 16 <= size; i += 16) {
        // Load 16 integers from each vector
        __m512i vec_a = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&a[i]));
        __m512i vec_b = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&b[i]));

        // Add the vectors
        __m512i vec_result = _mm512_add_epi32(vec_a, vec_b);

        // Store the result back into vector 'a'
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(&a[i]), vec_result);
    }

    // Handle any remaining elements
    for (; i < size; ++i) {
        a[i] += b[i];
    }
}

void parallelAddSse5(std::vector<int>& a, const std::vector<int>& b) {
    const size_t size = a.size();

    // Ensure both vectors are the same size
    if (b.size() != size) {
        throw std::invalid_argument("Vectors must be the same size");
    }

    size_t i = 0;

    // Process 8 integers at a time using AVX2 intrinsics
    for (; i + 8 <= size; i += 8) {
        // Load 8 integers from each vector
        __m256i vec_a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&a[i]));
        __m256i vec_b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&b[i]));

        // Add the vectors
        __m256i vec_result = _mm256_add_epi32(vec_a, vec_b);

        // Store the result back into vector 'a'
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(&a[i]), vec_result);
    }

    // Handle any remaining elements
    for (; i < size; ++i) {
        a[i] += b[i];
    }
}

// also fast, but not worth it for vectors that only have a size of 1000
void parallelAddSse6(std::vector<int>& a, const std::vector<int>& b) {
    const size_t size = a.size();

    // Ensure both vectors are the same size
    if (b.size() != size) {
        throw std::invalid_argument("Vectors must be the same size");
    }

    const size_t grain_size = 1024;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, size, grain_size),
        [&a, &b](const tbb::blocked_range<size_t>& range) {
            size_t i = range.begin();
            size_t end = range.end();

            // Process 16 integers at a time using AVX-512 intrinsics
            for (; i + 16 <= end; i += 16) {
                __m512i vec_a = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&a[i]));
                __m512i vec_b = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&b[i]));

                __m512i vec_result = _mm512_add_epi32(vec_a, vec_b);

                _mm512_storeu_si512(reinterpret_cast<__m512i*>(&a[i]), vec_result);
            }

            // Handle any remaining elements in the current range
            for (; i < end; ++i) {
                a[i] += b[i];
            }
        }
    );
}

void parallelAddSse7(std::vector<int>& a, const std::vector<int>& b) {
    const size_t size = a.size();

    // Handle any remaining elements
    for (size_t i = 0; i < size; ++i) {
        a[i] += b[i];
    }
}

void shuffle(std::vector<int>& list, RandomGenerator* iGenerator) {
    int n = list.size(); // The number of items left to shuffle (loop invariant).
    while (n > 1) {
        std::uniform_int_distribution<int> distribution(0, n - 1);
        int k = distribution(*iGenerator); // 0 <= k < n.
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

bool CPPSolutionBase::InSameClique(int nodeA, int nodeB) const
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

int CPPSolutionBase::calculateDistance(const CPPSolutionBase& iSolution) {
    int sum = 0;
    int nnode = mInstance->getNumberOfNodes();
    for (int i = 0; i < nnode; ++i) {
        for (int j = i + 1; j < nnode; ++j) {
            if (InSameClique(i, j)) {
                if (iSolution.InSameClique(i, j))
                    sum++;
            }
            else {
                if (iSolution.InSameClique(i, j))
                    sum++;
            }
        }
    }
    return sum;
}

void CPPSolutionBase::AddCandidate(const CPPCandidate& A) {};

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

    // reserve resource for time critical vectors for optimal performance
    mAllConnections.clear();
    mAllConnections.reserve(numVertices);

    for (size_t i = 0; i < numVertices; ++i)
    {
        std::vector<int> inner;
        inner.reserve(numVertices);
        mAllConnections.push_back(std::move(inner));
    }

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

bool CPPSolutionBase::CheckSolutionValid(CPPInstance& tInstance)
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
    if (nClique >= mAllConnections.size())
    {
        // Resize to ensure we have enough slots.
        mAllConnections.resize(nClique + 1);

        // Initialize the new clique row to the right size filled with zeros.
        mAllConnections[nClique].resize(mNodeClique.size(), 0);
    }

    std::vector<int>& newAllConnectedNode = mAllConnections[nClique];
    std::vector<int>& oldAllConnectedNode = mAllConnections[mNodeClique[nNode]];

    const std::vector<int>& NodeWeights = mInstance->getWeights()[nNode];
    const std::vector<int>& NodeNegativeWeights = mInstance->getNegativeWeights()[nNode];

    parallelAddSse7(newAllConnectedNode, NodeWeights);
    parallelAddSse7(oldAllConnectedNode, NodeNegativeWeights);
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

void CPPSolutionBase::SimulatedAnnealingSelectTrio(int n1, int n2, int n3, int& BestChange, std::vector<std::array<int, 2>>& BestRelocations)
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
    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];

    int n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    int n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    const auto& weights = mInstance->getWeights();
    int bWeight = weights[Relocation.mN0][Relocation.mN1];

    const int Size = static_cast<int>(mCliqueSizes.size());
    int mAllConn_n0Clique_n1 = mAllConnections[n0Clique][Relocation.mN1];
    bool sameCliques = (n1Clique == n0Clique);

    if (mCliqueSizes[n0Clique] > Relocation.mChange)
    {   
        // move n0 to new cluster
        Relocation.mChange = n0RemoveChange;
        Relocation.mC0 = Size;
        Relocation.mMoveType = SAMoveType::N0;
    }

    for (int c = 0; c < Size; c++)
    {
        const std::vector<int>& CliqueConnections = mAllConnections[c];

        int cChange0 = n0RemoveChange + CliqueConnections[Relocation.mN0];

        if (n0Clique != c && cChange0 > Relocation.mChange)
        {   
            // move n0 to cluster c
            Relocation.mChange = cChange0;
            Relocation.mC0 = c;
            Relocation.mMoveType = SAMoveType::N0;
        }

        int cChange1 = n1RemoveChange + CliqueConnections[Relocation.mN1];

        if (n1Clique != c && n0Clique != c)
        {
            int cChange = cChange1 + cChange0 + (sameCliques ? 2 * bWeight : bWeight);

            if (cChange > Relocation.mChange)
            {   
                // move n0 and n1 to cluster c
                Relocation.mChange = cChange;
                Relocation.mC0 = c;
                Relocation.mC1 = c;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }

        if (n0Clique != n1Clique && c != n0Clique)
        {
            int cChange;

            if (c != n1Clique)
            {
                cChange = cChange0 + n1RemoveChange + mAllConn_n0Clique_n1 - bWeight;
            }
            else
            {
                cChange = cChange0 + n1RemoveChange + mAllConn_n0Clique_n1 - 2 * bWeight;
            }

            if (cChange > Relocation.mChange)
            {   
                // move n0 to cluster c and n1 to cluster of n0
                Relocation.mChange = cChange;
                Relocation.mC0 = c;
                Relocation.mC1 = n0Clique;
                Relocation.mMoveType = SAMoveType::Slide;
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
    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];

    const int Size = static_cast<int>(mCliqueSizes.size());
    int mAllConn_n0Clique_n1 = mAllConnections[n0Clique][Relocation.mN1];
    bool sameCliques = (n1Clique == n0Clique);

    int n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    int n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    int bWeight = mInstance->getWeights()[Relocation.mN0][Relocation.mN1];

    if (n0RemoveChange < n1RemoveChange)
    {
        if (mCliqueSizes[n1Clique] > 1)
        {   
            // create new clique for n1
            Relocation.mChange = n1RemoveChange;
            Relocation.mC1 = Size;
            Relocation.mMoveType = SAMoveType::N1;
        }
    }
    else
    {
        if (mCliqueSizes[n0Clique] > 1)
        {   
            // create to clique for n0
            Relocation.mChange = n0RemoveChange;
            Relocation.mC0 = Size;
            Relocation.mMoveType = SAMoveType::N0;
        }
    }

    for (int c = 0; c < Size; c++)
    {
        const std::vector<int>& CliqueConnections = mAllConnections[c];

        int cChange0 = n0RemoveChange + CliqueConnections[Relocation.mN0];
        if (n0Clique != c && cChange0 > Relocation.mChange)
        {
            // move n0 to clique c
            Relocation.mChange = cChange0;
            Relocation.mC0 = c;
            Relocation.mMoveType = SAMoveType::N0;
        }
        int cChange1 = n1RemoveChange + CliqueConnections[Relocation.mN1];

        if (n1Clique != c && cChange1 > Relocation.mChange)
        {
            // move n1 to clique c
            Relocation.mChange = cChange1;
            Relocation.mC1 = c;
            Relocation.mMoveType = SAMoveType::N1;
        }

        if (n1Clique != c && n0Clique != c)
        {
            int cChange = cChange1 + cChange0 + (sameCliques ? 2 * bWeight : bWeight);

            if (cChange > Relocation.mChange)
            {
                // move n0 and n1 to clique c
                Relocation.mChange = cChange;
                Relocation.mC0 = c;
                Relocation.mC1 = c;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }

        if (n0Clique != n1Clique)
        {
            int cChange;

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
                    // slide n1 to c0 and n0 to c
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
                    // slide n0 to c1 and n1 to c
                    Relocation.mChange = cChange;
                    Relocation.mC0 = n1Clique;
                    Relocation.mC1 = c;
                    Relocation.mMoveType = SAMoveType::Slide;
                }
            }
        }
    }
}

void CPPSolutionBase::SASelectDualFull(SARelocation& Relocation, int weight, bool forceDualMove)
{   
    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];

    const int Size = static_cast<int>(mCliqueSizes.size());

    int n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    int n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    int bestN0Clique = n0Clique;
    int secondBestN0Clique = n0Clique;
    int bestN0Change = INT_MIN;
    int secondBestN0Change = INT_MIN;
    int bestN1Clique = n1Clique;
    int secondBestN1Clique = n1Clique;
    int bestN1Change = INT_MIN;
    int secondBestN1Change = INT_MIN;

    Relocation.mC0 = n0Clique;
    Relocation.mC1 = n1Clique;
    Relocation.mChange = INT_MIN;

    if (weight == 0) {
        weight = mInstance->getWeights()[Relocation.mN0][Relocation.mN1];
    }
    int mergeWeight = weight;
    int splitWeight = -weight;

    bool sameClique = n0Clique == n1Clique;
    if (sameClique) {
        mergeWeight = 0;
    } else {
        splitWeight = 0;
    }

    long cChange;
    const std::vector<int>& n0CliqueConnections = mAllConnections[n0Clique];
    const std::vector<int>& n1CliqueConnections = mAllConnections[n1Clique];

    for (int c = 0; c < Size; c++) {
        const std::vector<int>& cCliqueConnections = mAllConnections[c];
        int cChange0 = n0RemoveChange + cCliqueConnections[Relocation.mN0];
        int cChange1 = n1RemoveChange + cCliqueConnections[Relocation.mN1];

        if (n0Clique != c && n1Clique != c) {
            // move individually n0 to c
            if (cChange0 > bestN0Change) {
                secondBestN0Clique = bestN0Clique;
                bestN0Clique = c;
                secondBestN0Change = bestN0Change;
                bestN0Change = cChange0;
            }
            else if (cChange0 > secondBestN0Change) {
                secondBestN0Clique = c;
                secondBestN0Change = cChange0;
            }

            // move individually n1 to c
            if (cChange1 > bestN1Change) {
                secondBestN1Clique = bestN1Clique;
                bestN1Clique = c;
                secondBestN1Change = bestN1Change;
                bestN1Change = cChange1;
            }
            else if (cChange1 > secondBestN1Change) {
                secondBestN1Clique = c;
                secondBestN1Change = cChange1;
            }

            // move both to c
            cChange = cChange1 + cChange0 + mergeWeight - 2 * splitWeight;
            if (cChange > Relocation.mChange) {
                Relocation.mC0 = c;
                Relocation.mC1 = c;
                Relocation.mChange = cChange;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }

        if (!sameClique) {
            if (n0Clique != c) {
                // slide n1 to n0Clique and n0 to c
                if (c != n1Clique) {
                    cChange = cChange0 + n1RemoveChange + n0CliqueConnections[Relocation.mN1] - mergeWeight;
                } else {
                    cChange = cChange0 + n1RemoveChange + n0CliqueConnections[Relocation.mN1] - 2 * mergeWeight;
                }

                if (cChange > Relocation.mChange) {
                    Relocation.mC0 = c;
                    Relocation.mC1 = n0Clique;
                    Relocation.mChange = cChange;
                    Relocation.mMoveType = SAMoveType::Slide;
                    }
            } else {
                // move n1 to n0Clique
                if (cChange1 > Relocation.mChange) {
                    Relocation.mC0 = n0Clique;
                    Relocation.mC1 = n0Clique;
                    Relocation.mChange = cChange1;
                    Relocation.mMoveType = SAMoveType::N1;
                }
            }

            if (n1Clique != c) {
                // slide n0 to n1Clique and n1 to c
                if (c != n0Clique) {
                    cChange = cChange1 + n0RemoveChange + n1CliqueConnections[Relocation.mN0] - mergeWeight;
                }
                else {
                    cChange = cChange1 + n0RemoveChange + n1CliqueConnections[Relocation.mN0] - 2 * mergeWeight;
                }

                if (cChange > Relocation.mChange) {
                    Relocation.mC0 = n1Clique;
                    Relocation.mC1 = c;
                    Relocation.mChange = cChange;
                    Relocation.mMoveType = SAMoveType::Slide;
                    }
            } else {
                // move n0 to n1Clique
                if (cChange0 > Relocation.mChange) {
                    Relocation.mC0 = n1Clique;
                    Relocation.mC1 = n1Clique;
                    Relocation.mChange = cChange0;
                    Relocation.mMoveType = SAMoveType::N0;
                }
            }
        }
    }

    // remove individually
    if (n0RemoveChange > bestN0Change) {
        secondBestN0Clique = bestN0Clique;
        bestN0Clique = Size;
        secondBestN0Change = bestN0Change;
        bestN0Change = n0RemoveChange;
    }
    if (n1RemoveChange > bestN1Change) {
        secondBestN1Clique = bestN1Clique;
        bestN1Clique = Size;
        secondBestN1Change = bestN1Change;
        bestN1Change = n1RemoveChange;
    }

    // combine individual moves to together moves
    if (bestN0Clique != bestN1Clique) {
        // individual movement of nodes
        cChange = bestN0Change + bestN1Change - splitWeight;
        if (cChange > Relocation.mChange) {
            Relocation.mC0 = bestN0Clique;
            Relocation.mC1 = bestN1Clique;
            Relocation.mChange = cChange;
            Relocation.mMoveType = SAMoveType::Both;
        }
    } else {
        // conflict in movement of nodes
        if (bestN0Clique == Size) {
            // move to newly created cluster
            cChange = bestN0Change + bestN1Change - splitWeight;
            if (cChange > Relocation.mChange) {
                Relocation.mC0 = Size;
                Relocation.mC1 = Size + 1;
                Relocation.mChange = cChange;
                Relocation.mMoveType = SAMoveType::Both;
            }
                }
                else {
            cChange = bestN0Change + secondBestN1Change - splitWeight;
            if (secondBestN1Change != INT_MIN && cChange > Relocation.mChange) {
                Relocation.mC0 = bestN0Clique;
                Relocation.mC1 = secondBestN1Clique;
                Relocation.mChange = cChange;
                Relocation.mMoveType = SAMoveType::Both;
                }
            cChange = secondBestN0Change + bestN1Change - splitWeight;
            if (secondBestN0Change != INT_MIN && cChange > Relocation.mChange) {
                Relocation.mC0 = secondBestN0Clique;
                Relocation.mC1 = bestN1Clique;
                Relocation.mChange = cChange;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }
    }

    // remove together
    int removeTogether = n0RemoveChange + n1RemoveChange + mergeWeight - 2 * splitWeight;
    if (removeTogether > Relocation.mChange) {
        Relocation.mC0 = Size;
        Relocation.mC1 = Size;
        Relocation.mChange = removeTogether;
        Relocation.mMoveType = SAMoveType::Both;
    }

    // check if better if one node does not move
    if (bestN0Change > Relocation.mChange) {
        // split up and return two Relocations instead
        // four attributes: nextAcceptRelocation, nextDenyRelocation, nextRelocationCalculated, prevAccepted
        Relocation.mC0 = bestN0Clique;
        Relocation.mC1 = n1Clique;
        Relocation.mChange = bestN0Change;
        Relocation.mMoveType = SAMoveType::N0;
    }
    if (bestN1Change > Relocation.mChange) {
        // split up and return two Relocations instead
        Relocation.mC0 = n0Clique;
        Relocation.mC1 = bestN1Clique;
        Relocation.mChange = bestN1Change;
        Relocation.mMoveType = SAMoveType::N1;
    }

    
    if (forceDualMove) {
        if (Relocation.mMoveType == SAMoveType::N0) {
            nextAcceptRelocation.mN0 = Relocation.mN0;
            nextAcceptRelocation.mN1 = Relocation.mN1;
            nextAcceptRelocation.mC0 = Relocation.mC0;
            nextAcceptRelocation.mMoveType = SAMoveType::N1;
            nextDenyRelocation.mN0 = Relocation.mN0;
            nextDenyRelocation.mN1 = Relocation.mN1;
            nextDenyRelocation.mC0 = n0Clique;
            nextDenyRelocation.mMoveType = SAMoveType::N1;
            nextRelocationCalculated = true;
            int acceptCChange;
            int denyCChange;
            if (Relocation.mC0 == n1Clique) {
                // n0 is moved to c1
                nextAcceptRelocation.mC1 = bestN1Clique;
                nextAcceptRelocation.mChange = bestN1Change - mergeWeight;
                nextDenyRelocation.mC1 = bestN1Clique;
                nextDenyRelocation.mChange = bestN1Change;
            }
            else {
                // n0 is moved somewhere else than c1
                int acceptBest, denyBest, acceptSecondBest, denySecondBest;
                if (bestN1Clique == Relocation.mC0) {
                    // best clique for n1 is the one n0 is moved to
                    acceptBest = bestN1Change - splitWeight + weight;
                    denyBest = bestN1Change;
                }
                else {
                    // best clique for n1 is NOT the one n0 is moved to
                    acceptBest = bestN1Change - splitWeight;
                    denyBest = bestN1Change;
                }
                if (secondBestN1Clique == Relocation.mC0) {
                    // second best clique for n1 is the one n0 is moved to
                    acceptSecondBest = secondBestN1Change - 2 * splitWeight + mergeWeight;
                    denySecondBest = secondBestN1Change;
                }
                else {
                    // second best clique for n1 is NOT the one n0 is moved to
                    acceptSecondBest = secondBestN1Change - splitWeight;
                    denySecondBest = secondBestN1Change;
                }

                if (acceptBest >= acceptSecondBest) {
                    nextAcceptRelocation.mC1 = bestN1Clique;
                    nextAcceptRelocation.mChange = acceptBest;
                }
                else {
                    nextAcceptRelocation.mC1 = secondBestN1Clique;
                    nextAcceptRelocation.mChange = acceptSecondBest;
                }
                if (denyBest >= denySecondBest) {
                    nextDenyRelocation.mC1 = bestN1Clique;
                    nextDenyRelocation.mChange = denyBest;
                }
                else {
                    nextDenyRelocation.mC1 = secondBestN1Clique;
                    nextDenyRelocation.mChange = denySecondBest;
                }
            }
        } else if (Relocation.mMoveType == SAMoveType::N1) {
            nextAcceptRelocation.mN0 = Relocation.mN0;
            nextAcceptRelocation.mN1 = Relocation.mN1;
            nextAcceptRelocation.mC1 = Relocation.mC1;
            nextAcceptRelocation.mMoveType = SAMoveType::N0;
            nextDenyRelocation.mN0 = Relocation.mN0;
            nextDenyRelocation.mN1 = Relocation.mN1;
            nextDenyRelocation.mC1 = n1Clique;
            nextDenyRelocation.mMoveType = SAMoveType::N0;
            nextRelocationCalculated = true;
            int acceptCChange;
            int denyCChange;
            if (Relocation.mC1 == n0Clique) {
                // n1 is moved to c0
                nextAcceptRelocation.mC0 = bestN0Clique;
                nextAcceptRelocation.mChange = bestN0Change - mergeWeight;
                nextDenyRelocation.mC0 = bestN0Clique;
                nextDenyRelocation.mChange = bestN0Change;
            }
            else {
                // n1 is moved somewhere else than c0
                int acceptBest, denyBest, acceptSecondBest, denySecondBest;
                if (bestN0Clique == Relocation.mC1) {
                    // best clique for n0 is the one n1 is moved to
                    acceptBest = bestN0Change - splitWeight + weight;
                    denyBest = bestN0Change;
                }
                else {
                    // best clique for n0 is NOT the one n1 is moved to
                    acceptBest = bestN0Change - splitWeight;
                    denyBest = bestN0Change;
                }
                if (secondBestN0Clique == Relocation.mC1) {
                    // second best clique for n0 is the one n1 is moved to
                    acceptSecondBest = secondBestN0Change - 2 * splitWeight + mergeWeight;
                    denySecondBest = secondBestN0Change;
                }
                else {
                    // second best clique for n0 is NOT the one n1 is moved to
                    acceptSecondBest = secondBestN0Change - splitWeight;
                    denySecondBest = secondBestN0Change;
                }

                if (acceptBest >= acceptSecondBest) {
                    nextAcceptRelocation.mC0 = bestN0Clique;
                    nextAcceptRelocation.mChange = acceptBest;
                }
                else {
                    nextAcceptRelocation.mC0 = secondBestN0Clique;
                    nextAcceptRelocation.mChange = acceptSecondBest;
                }
                if (denyBest >= denySecondBest) {
                    nextDenyRelocation.mC0 = bestN0Clique;
                    nextDenyRelocation.mChange = denyBest;
                }
                else {
                    nextDenyRelocation.mC0 = secondBestN0Clique;
                    nextDenyRelocation.mChange = denySecondBest;
                }
            }
        }
    }


    // check if better if no node moves
    // commented because backward moves should be allowed
    /*
    if (Relocation.mChange < 0) {
        Relocation.mC0 = n0Clique;
        Relocation.mC1 = n1Clique;
        Relocation.mChange = 0;
        Relocation.mMoveType = SAMoveType::None;
    }*/

    // assert(Relocation.mChange >= 0);
}

void CPPSolutionBase::SASelectDualSplit(SARelocation& Relocation, int weight, bool forceDualMove)
{
    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];

    const int Size = static_cast<int>(mCliqueSizes.size());

    int n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    int n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    SASingleRelocation bestN0Relocation;
    bestN0Relocation.node = Relocation.mN0;
    bestN0Relocation.clique = n0Clique;
    bestN0Relocation.change = INT_MIN;
    SASingleRelocation secondBestN0Relocation;
    secondBestN0Relocation.node = Relocation.mN0;
    secondBestN0Relocation.clique = n0Clique;
    secondBestN0Relocation.change = INT_MIN;
    SASingleRelocation bestN1Relocation;
    bestN1Relocation.node = Relocation.mN1;
    bestN1Relocation.clique = n1Clique;
    bestN1Relocation.change = INT_MIN;
    SASingleRelocation secondBestN1Relocation;
    secondBestN1Relocation.node = Relocation.mN1;
    secondBestN1Relocation.clique = n1Clique;
    secondBestN1Relocation.change = INT_MIN;

    SADualSplitRelocation bestCombinedRelocation;
    bestCombinedRelocation.mN0 = Relocation.mN0;
    bestCombinedRelocation.mN1 = Relocation.mN1;
    bestCombinedRelocation.mC0 = n0Clique;
    bestCombinedRelocation.mC1 = n1Clique;
    bestCombinedRelocation.mChange0 = INT_MIN;
    bestCombinedRelocation.mChange1 = INT_MIN;
    bestCombinedRelocation.mChange = INT_MIN;

    if (weight == 0) {
        weight = mInstance->getWeights()[Relocation.mN0][Relocation.mN1];
    }
    int mergeWeight = weight;
    int splitWeight = -weight;

    bool sameClique = n0Clique == n1Clique;
    if (sameClique) {
        mergeWeight = 0;
    }
    else {
        splitWeight = 0;
    }

    long cChange;
    const std::vector<int>& n0CliqueConnections = mAllConnections[n0Clique];
    const std::vector<int>& n1CliqueConnections = mAllConnections[n1Clique];

    for (int c = 0; c < Size; c++) {
        const std::vector<int>& cCliqueConnections = mAllConnections[c];
        int cChange0 = n0RemoveChange + cCliqueConnections[Relocation.mN0];
        int cChange1 = n1RemoveChange + cCliqueConnections[Relocation.mN1];

        if (n0Clique != c && n1Clique != c) {
            // move individually n0 to c
            if (cChange0 > bestN0Relocation.change) {
                secondBestN0Relocation.clique = bestN0Relocation.clique;
                bestN0Relocation.clique = c;
                secondBestN0Relocation.change = bestN0Relocation.change;
                bestN0Relocation.change = cChange0;
            }
            else if (cChange0 > secondBestN0Relocation.change) {
                secondBestN0Relocation.clique = c;
                secondBestN0Relocation.change = cChange0;
            }

            // move individually n1 to c
            if (cChange1 > bestN1Relocation.change) {
                secondBestN1Relocation.clique = bestN1Relocation.clique;
                bestN1Relocation.clique = c;
                secondBestN1Relocation.change = bestN1Relocation.change;
                bestN1Relocation.change = cChange1;
            }
            else if (cChange1 > secondBestN1Relocation.change) {
                secondBestN1Relocation.clique = c;
                secondBestN1Relocation.change = cChange1;
            }

            // move both to c
            cChange = cChange1 + cChange0 + mergeWeight - 2 * splitWeight;
            if (cChange > Relocation.mChange) {
                bestCombinedRelocation.mC0 = c;
                bestCombinedRelocation.mC1 = c;
                bestCombinedRelocation.mChange = cChange;
                if (cChange0 > cChange1) {
                    bestCombinedRelocation.mChange0 = cChange0;
                    bestCombinedRelocation.mChange1 = cChange1 + mergeWeight - 2 * splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    bestCombinedRelocation.mChange0 = cChange0 + mergeWeight - 2 * splitWeight;
                    bestCombinedRelocation.mChange1 = cChange1;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
        }

        if (!sameClique) {
            if (n0Clique != c) {
                int tempChange1 = n1RemoveChange + n0CliqueConnections[Relocation.mN1];
                int mergeAdjustment;

                // slide n1 to n0Clique and n0 to c
                if (c != n1Clique) {
                    mergeAdjustment = -mergeWeight;
                }
                else {
                    mergeAdjustment = -2 * mergeWeight;
                }

                cChange = cChange0 + tempChange1 + mergeAdjustment;

                if (cChange > Relocation.mChange) {
                    bestCombinedRelocation.mC0 = c;
                    bestCombinedRelocation.mC1 = n0Clique;
                    bestCombinedRelocation.mChange = cChange;
                    if (cChange0 > tempChange1) {
                        bestCombinedRelocation.mChange0 = cChange0;
                        bestCombinedRelocation.mChange1 = tempChange1 + mergeAdjustment;
                        bestCombinedRelocation.moveN0First = true;
                    }
                    else {
                        bestCombinedRelocation.mChange0 = cChange0 + mergeAdjustment;
                        bestCombinedRelocation.mChange1 = tempChange1;
                        bestCombinedRelocation.moveN0First = false;
                    }
                }
            }

            if (n1Clique != c) {
                int tempChange0 = n0RemoveChange + n1CliqueConnections[Relocation.mN0];
                int mergeAdjustment;

                // slide n0 to n1Clique and n1 to c
                if (c != n0Clique) {
                    mergeAdjustment = -mergeWeight;
                }
                else {
                    mergeAdjustment = -2 * mergeWeight;
                }

                cChange = cChange1 + tempChange0 + mergeAdjustment;

                if (cChange > Relocation.mChange) {
                    bestCombinedRelocation.mC0 = n1Clique;
                    bestCombinedRelocation.mC1 = c;
                    bestCombinedRelocation.mChange = cChange;
                    if (cChange1 > tempChange0) {
                        bestCombinedRelocation.mChange0 = tempChange0 + mergeAdjustment;
                        bestCombinedRelocation.mChange1 = cChange1;
                        bestCombinedRelocation.moveN0First = false;
                    }
                    else {
                        bestCombinedRelocation.mChange0 = tempChange0;
                        bestCombinedRelocation.mChange1 = cChange1 + mergeAdjustment;
                        bestCombinedRelocation.moveN0First = true;
                    }
                }
            }
        }
    }

    // remove individually
    if (n0RemoveChange > bestN0Relocation.change) {
        secondBestN0Relocation.clique = bestN0Relocation.clique;
        bestN0Relocation.clique = Size;
        secondBestN0Relocation.change = bestN0Relocation.change;
        bestN0Relocation.change = n0RemoveChange;
    }
    if (n1RemoveChange > bestN1Relocation.change) {
        secondBestN1Relocation.clique = bestN1Relocation.clique;
        bestN1Relocation.clique = Size;
        secondBestN1Relocation.change = bestN1Relocation.change;
        bestN1Relocation.change = n1RemoveChange;
    }

    // combine individual moves to together moves
    if (bestN0Relocation.clique != bestN1Relocation.clique) {
        // individual movement of nodes
        cChange = bestN0Relocation.change + bestN1Relocation.change - splitWeight;
        if (cChange > bestCombinedRelocation.mChange) {
            bestCombinedRelocation.mC0 = bestN0Relocation.clique;
            bestCombinedRelocation.mC1 = bestN1Relocation.clique;
            bestCombinedRelocation.mChange = cChange;
            if (bestN0Relocation.change > bestN1Relocation.change) {
                bestCombinedRelocation.mChange0 = bestN0Relocation.change;
                bestCombinedRelocation.mChange1 = bestN1Relocation.change - splitWeight;
                bestCombinedRelocation.moveN0First = true;
            }
            else {
                bestCombinedRelocation.mChange0 = bestN0Relocation.change - splitWeight;
                bestCombinedRelocation.mChange1 = bestN1Relocation.change;
                bestCombinedRelocation.moveN0First = false;
            }
        }
    }
    else {
        // conflict in movement of nodes
        if (bestN0Relocation.clique == Size) {
            // move to newly created cluster
            cChange = bestN0Relocation.change + bestN1Relocation.change - splitWeight;
            if (cChange > bestCombinedRelocation.mChange) {
                bestCombinedRelocation.mChange = cChange;
                if (bestN0Relocation.change > bestN1Relocation.change) {
                    bestCombinedRelocation.mC0 = Size;
                    bestCombinedRelocation.mC1 = Size + 1;
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change - splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    bestCombinedRelocation.mC0 = Size + 1;
                    bestCombinedRelocation.mC1 = Size;
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change - splitWeight;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
        }
        else {
            cChange = bestN0Relocation.change + secondBestN1Relocation.change - splitWeight;
            if (secondBestN1Relocation.change != INT_MIN && cChange > bestCombinedRelocation.mChange) {
                bestCombinedRelocation.mC0 = bestN0Relocation.clique;
                bestCombinedRelocation.mC1 = secondBestN1Relocation.clique;
                bestCombinedRelocation.mChange = cChange;
                if (bestN0Relocation.change > secondBestN1Relocation.change) {
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change;
                    bestCombinedRelocation.mChange1 = secondBestN1Relocation.change - splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change - splitWeight;
                    bestCombinedRelocation.mChange1 = secondBestN1Relocation.change;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
            cChange = secondBestN0Relocation.change + bestN1Relocation.change - splitWeight;
            if (secondBestN0Relocation.change != INT_MIN && cChange > bestCombinedRelocation.mChange) {
                bestCombinedRelocation.mC0 = secondBestN0Relocation.clique;
                bestCombinedRelocation.mC1 = bestN1Relocation.clique;
                bestCombinedRelocation.mChange = cChange;
                if (secondBestN0Relocation.change > bestN1Relocation.change) {
                    bestCombinedRelocation.mChange0 = secondBestN0Relocation.change;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change - splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    bestCombinedRelocation.mChange0 = secondBestN0Relocation.change - splitWeight;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
        }
    }

    // remove together
    int removeTogether = n0RemoveChange + n1RemoveChange + mergeWeight - 2 * splitWeight;
    if (removeTogether > bestCombinedRelocation.mChange) {
        bestCombinedRelocation.mC0 = Size;
        bestCombinedRelocation.mC1 = Size;
        bestCombinedRelocation.mChange = removeTogether;
        if (n0RemoveChange > n1RemoveChange) {
            bestCombinedRelocation.mChange0 = n0RemoveChange;
            bestCombinedRelocation.mChange1 = n1RemoveChange + mergeWeight - 2 * splitWeight;
            bestCombinedRelocation.moveN0First = true;
        }
        else {
            bestCombinedRelocation.mChange0 = n0RemoveChange + mergeWeight - 2 * splitWeight;
            bestCombinedRelocation.mChange1 = n1RemoveChange;
            bestCombinedRelocation.moveN0First = false;
        }
    }

    Relocation.mN0 = bestCombinedRelocation.mN0;
    Relocation.mC0 = bestCombinedRelocation.mC0;
    Relocation.mN1 = bestCombinedRelocation.mN1;
    Relocation.mC1 = bestCombinedRelocation.mC1;
    nextAcceptRelocation.copy(Relocation);
    nextDenyRelocation.mN0 = bestN0Relocation.node;
    nextDenyRelocation.mC0 = bestN0Relocation.clique;
    nextDenyRelocation.mN1 = bestN1Relocation.node;
    nextDenyRelocation.mC1 = bestN1Relocation.clique;

    if (bestCombinedRelocation.moveN0First) {
        Relocation.mChange = bestCombinedRelocation.mChange0;
        Relocation.mProbChange = bestCombinedRelocation.mChange0;
        Relocation.mMoveType = SAMoveType::N0;

        nextAcceptRelocation.mChange = bestCombinedRelocation.mChange1;
        nextAcceptRelocation.mProbChange = bestCombinedRelocation.mChange1;
        nextAcceptRelocation.mMoveType = SAMoveType::N1;
        /*
        nextDenyRelocation.copy(Relocation);
        nextDenyRelocation.mN0 = bestN1Relocation.node;
        nextDenyRelocation.mC0 = bestN1Relocation.clique;
        nextDenyRelocation.mChange = bestN1Relocation.change;*/

        // choose best alternative relocation
        /*
        if (bestN0Relocation.change > bestN1Relocation.change && bestN0Relocation.change > Relocation.mChange) {
            nextDenyRelocation.mMoveType = SAMoveType::N0;
            nextDenyRelocation.mChange = bestN0Relocation.change;
            nextDenyRelocation.mProbChange = bestN0Relocation.change;
        }
        else if (secondBestN0Relocation.change > bestN1Relocation.change && secondBestN0Relocation.change > Relocation.mChange) {
            nextDenyRelocation.mMoveType = SAMoveType::N0;
            nextDenyRelocation.mChange = secondBestN0Relocation.change;
            nextDenyRelocation.mProbChange = secondBestN0Relocation.change;
        }
        else {
            nextDenyRelocation.mMoveType = SAMoveType::N1;
            nextDenyRelocation.mChange = bestN1Relocation.change;
            nextDenyRelocation.mProbChange = bestN1Relocation.change;
        }*/
        
        nextDenyRelocation.mMoveType = SAMoveType::N1;
        nextDenyRelocation.mChange = bestN1Relocation.change;
        nextDenyRelocation.mProbChange = bestN1Relocation.change;
    }
    else {
        Relocation.mChange = bestCombinedRelocation.mChange1;
        Relocation.mProbChange = bestCombinedRelocation.mChange1;
        Relocation.mMoveType = SAMoveType::N1;

        nextAcceptRelocation.mChange = bestCombinedRelocation.mChange0;
        nextAcceptRelocation.mProbChange = bestCombinedRelocation.mChange0;
        nextAcceptRelocation.mMoveType = SAMoveType::N0;
        /*
        nextDenyRelocation.copy(Relocation);
        nextDenyRelocation.mN1 = bestN0Relocation.node;
        nextDenyRelocation.mC1 = bestN0Relocation.clique;
        nextDenyRelocation.mChange = bestN0Relocation.change;*/

        // choose best alternative relocation
        /*
        if (bestN1Relocation.change > bestN0Relocation.change && bestN1Relocation.change > Relocation.mChange) {
            nextDenyRelocation.mMoveType = SAMoveType::N1;
            nextDenyRelocation.mChange = bestN1Relocation.change;
            nextDenyRelocation.mProbChange = bestN1Relocation.change;
        }
        else if (secondBestN1Relocation.change > bestN0Relocation.change && secondBestN1Relocation.change > Relocation.mChange) {
            nextDenyRelocation.mMoveType = SAMoveType::N1;
            nextDenyRelocation.mChange = secondBestN1Relocation.change;
            nextDenyRelocation.mProbChange = secondBestN1Relocation.change;
        }
        else {
            nextDenyRelocation.mMoveType = SAMoveType::N0;
            nextDenyRelocation.mChange = bestN0Relocation.change;
            nextDenyRelocation.mProbChange = bestN0Relocation.change;
        }*/

        
        nextDenyRelocation.mMoveType = SAMoveType::N0;
        nextDenyRelocation.mChange = bestN0Relocation.change;
        nextDenyRelocation.mProbChange = bestN0Relocation.change;
    }

    // what if nextDenyRelocation.mChange > Relocation.mChange + nextAcceptRelocation.mChange && nextDenyRelocation.mChange > Relocation.mChange -> yes possible
    //      then: return nextDenyRelocation as current relocation, (but only if change is positive?)
    //      also consider best relocation of first node?
    //      is it possible the best relocation of first node is better than next deny relocation? -> catch this case
    
    // decide to split or bind the double move depending on score

    nextRelocationCalculated = true;
    // nextAcceptRelocation.forceAccept = false;
    /*
    Relocation.forceAccept = false;
    if (Relocation.mChange < Relocation.mChange + nextAcceptRelocation.mChange) {
        Relocation.mChange = Relocation.mChange + nextAcceptRelocation.mChange;
        Relocation.mProbChange = Relocation.mChange;
        if (Relocation.mC1 == Size && Relocation.mC0 == Size + 1) {
            Relocation.mC0 = Size;
            Relocation.mC1 == Size + 1;
        }
        Relocation.mMoveType = SAMoveType::Both;
        // nextAcceptRelocation.forceAccept = true;
        nextRelocationCalculated = false;
    }*/

    

    // decide if alternative relocation is better than double relocation
    /*
    if (nextDenyRelocation.mChange > Relocation.mChange && nextDenyRelocation.mChange > Relocation.mChange + nextAcceptRelocation.mChange) {
        Relocation.copy(nextDenyRelocation);
        nextRelocationCalculated = false;
    }*/
    
    if (nextAcceptRelocation.mChange > 0) {
        Relocation.mProbChange = Relocation.mChange + nextAcceptRelocation.mChange;
    }
    else {
        Relocation.mProbChange = Relocation.mChange;
    }
}

void CPPSolutionBase::SASelectSingle(SARelocation& Relocation)
{

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
        const std::vector<int>& CliqueConnections = mAllConnections[c];

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

void CPPSolutionBase::SASelectSingleEdge(SARelocation& Relocation, bool forceDualMove) {
    if (nextRelocationCalculated) {
        nextRelocationCalculated = false;
        if (prevAccepted) {
            Relocation.copy(nextAcceptRelocation);
        }
        else {
            Relocation.copy(nextDenyRelocation);
        }
        return;
    }

    const auto& edge = mInstance->getRandEdge(*mGenerator);

    int n0 = edge[0];
    int n1 = edge[1];
    int weight = edge[2];

    Relocation.mN0 = n0;
    Relocation.mN1 = n1;

    SASelectDualSplit(Relocation, weight, forceDualMove);
}

void CPPSolutionBase::SASelectDualR(SARelocation& Relocation)
{   
    
    int n0, n1;
    
    std::uniform_int_distribution<int> distribution(0, mInstance->getNumberOfNodes() - 1);
    n0 = distribution(*mGenerator);

    Relocation.mN1 = Relocation.mN0;
    Relocation.mN0 = n0;

    if (Relocation.mN0 != Relocation.mN1)
        SASelectDual(Relocation);
    else
        SASelectSingle(Relocation);

    Relocation.mProbChange = Relocation.mChange;
}


void CPPSolutionBase::SASelectDualNeighborOne(SARelocation& Relocation)
{
    int n0, n1;

    int neighborSize = 0;
    while (neighborSize == 0) {
        std::uniform_int_distribution<int> distribution(0, mInstance->getNumberOfNodes() - 1);
        n1 = distribution(*mGenerator);
        neighborSize = mInstance->getNeighborSize()[n1];
    }

    int i = (*mGenerator)() % neighborSize;
    n0 = mInstance->getNeighbors()[n1][i];

    Relocation.mN1 = n1;
    Relocation.mN0 = n0;

    int n0Clique = mNodeClique[Relocation.mN0];
    int n1Clique = mNodeClique[Relocation.mN1];

    const int Size = static_cast<int>(mCliqueSizes.size());
    int mAllConn_n0Clique_n1 = mAllConnections[n0Clique][Relocation.mN1];
    bool sameCliques = (n1Clique == n0Clique);

    int n0RemoveChange = -mAllConnections[n0Clique][Relocation.mN0];
    int n1RemoveChange = -mAllConnections[n1Clique][Relocation.mN1];

    int bWeight = mInstance->getWeights()[Relocation.mN0][Relocation.mN1];

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
        const std::vector<int>& CliqueConnections = mAllConnections[c];

        int cChange0 = n0RemoveChange + CliqueConnections[Relocation.mN0];
        if (n0Clique != c && cChange0 > Relocation.mChange)
        {   
            // move n0 to cluster c
            Relocation.mChange = cChange0;
            Relocation.mC0 = c;
            Relocation.mMoveType = SAMoveType::N0;
        }
        int cChange1 = n1RemoveChange + CliqueConnections[Relocation.mN1];

        if (n1Clique != c && cChange1 > Relocation.mChange)
        {   
            // move n1 to cluster c
            Relocation.mChange = cChange1;
            Relocation.mC1 = c;
            Relocation.mMoveType = SAMoveType::N1;
        }

        if (n1Clique != c && n0Clique != c)
        {
            int cChange = cChange1 + cChange0 + (sameCliques ? 2 * bWeight : bWeight);

            if (cChange > Relocation.mChange)
            {
                // move n0 and n1 to cluster c
                Relocation.mChange = cChange;
                Relocation.mC0 = c;
                Relocation.mC1 = c;
                Relocation.mMoveType = SAMoveType::Both;
            }
        }

        if (n0Clique != n1Clique && c != n0Clique)
        {
            int cChange;

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
}

void CPPSolutionBase::SASelectSingleR(SARelocation& Relocation)
{
    int n0, n1;

    std::uniform_int_distribution<int> distribution(0, mInstance->getNumberOfNodes() - 1);
    n0 = distribution(*mGenerator);

    Relocation.mN0 = n0;

    SASelectSingle(Relocation);

    Relocation.mProbChange = Relocation.mChange;
}

// Helper to update the best (and second best) single relocations.
inline void updateBestAndSecondBest(SASingleRelocationStruct& best,
    SASingleRelocationStruct& secondBest,
    int node, int newClique, int newChange)
{
    if (newChange > best.change) {
        // Promote old best to second best
        secondBest = best;
        // New best
        best.node = node;
        best.clique = newClique;
        best.change = newChange;
    }
    else if (newChange > secondBest.change) {
        secondBest.node = node;
        secondBest.clique = newClique;
        secondBest.change = newChange;
    }
}

// Helper to update bestCombined if we find a better sum change.
inline void updateBestCombined(SADualSplitRelocationStruct& bestCombined,
    int newC0, int newC1,
    int newChange
)
{
    if (newChange > bestCombined.mChange) {
        bestCombined.mC0 = newC0;
        bestCombined.mC1 = newC1;
        bestCombined.mChange = newChange;
    }
}

void CPPSolutionBase::SASelectDoubleR(SARelocationStruct& RelocationBoth,
    SARelocationStruct& RelocationN0,
    SARelocationStruct& RelocationN1)
{
    // 1. Pick a random edge
    std::uniform_int_distribution<int> distribution(0, mInstance->getNumberOfEdges() - 1);
    const auto& edge = mInstance->getEdges()[distribution(*mGenerator)];
    // const auto& edge = mInstance->getSampledRandEdge();
    int n0 = edge[0];
    int n1 = edge[1];
    int weight = edge[2];

    if (weight == 0) {
        weight = mInstance->getWeights()[n0][n1];
    }

    // 2. Prepare relocation data
    RelocationBoth.mN0 = n0;
    RelocationBoth.mN1 = n1;

    int n0Clique = mNodeClique[n0];
    int n1Clique = mNodeClique[n1];

    const int size = static_cast<int>(mCliqueSizes.size());

    // Precompute remove changes
    int n0RemoveChange = -mAllConnections[n0Clique][n0];
    int n1RemoveChange = -mAllConnections[n1Clique][n1];

    // Initialize best & second-best single relocations
    SASingleRelocationStruct bestN0{ n0, n0Clique, INT_MIN };
    SASingleRelocationStruct secondN0{ n0, n0Clique, INT_MIN };
    SASingleRelocationStruct bestN1{ n1, n1Clique, INT_MIN };
    SASingleRelocationStruct secondN1{ n1, n1Clique, INT_MIN };

    // Combined relocation
    SADualSplitRelocationStruct bestCombined;
    bestCombined.mN0 = n0;
    bestCombined.mN1 = n1;
    bestCombined.mC0 = n0Clique;
    bestCombined.mC1 = n1Clique;
    bestCombined.mChange = INT_MIN;

    // If they are in the same clique, merging them is effectively 0 cost;
    // if in different cliques, splitting them is 0 cost.
    bool sameClique = (n0Clique == n1Clique);
    int mergeWeight = sameClique ? 0 : weight;
    int splitWeight = sameClique ? -weight : 0;

    // 3. Main loop over possible cliques
    for (int c = 0; c < size; ++c) {
        // Single-move changes to clique c
        const std::vector<int>& connC = mAllConnections[c];
        int cChange0 = n0RemoveChange + connC[n0];
        int cChange1 = n1RemoveChange + connC[n1];

        // 3a. Update best single relocations if c differs from the current clique
        if (c != n0Clique) {
            updateBestAndSecondBest(bestN0, secondN0, n0, c, cChange0);
        }
        if (c != n1Clique) {
            updateBestAndSecondBest(bestN1, secondN1, n1, c, cChange1);
        }

        // 3b. Move both to the same new clique c
        if (c != n0Clique && c != n1Clique) {
            int cBoth = cChange0 + cChange1 + mergeWeight - 2 * splitWeight;
            // We only care about the sum
            updateBestCombined(bestCombined, c, c, cBoth);
        }

        // 3c. If they are in different cliques, consider "sliding" one into the other's clique
        if (!sameClique) {
            // n1 -> n0Clique, n0 -> c
            if (n0Clique != c) {
                int tempChange1 = n1RemoveChange + mAllConnections[n0Clique][n1];
                // If c != n1Clique, reduce once by mergeWeight; else reduce twice
                int mergeAdjustment = (c != n1Clique) ? -mergeWeight : -2 * mergeWeight;
                int cChangeSlide = (cChange0 + tempChange1 + mergeAdjustment);

                updateBestCombined(bestCombined,
                    c,          // new clique for n0
                    n0Clique,   // new clique for n1
                    cChangeSlide);
            }

            // n0 -> n1Clique, n1 -> c
            if (n1Clique != c) {
                int tempChange0 = n0RemoveChange + mAllConnections[n1Clique][n0];
                int mergeAdjustment = (c != n0Clique) ? -mergeWeight : -2 * mergeWeight;
                int cChangeSlide = (tempChange0 + cChange1 + mergeAdjustment);

                updateBestCombined(bestCombined,
                    n1Clique,   // new clique for n0
                    c,          // new clique for n1
                    cChangeSlide);
            }
        }
    } // end for (c)

    // 4. Consider removing n0 and n1 individually (assign them to a “new” cluster = size)
    if (n0RemoveChange > bestN0.change) {
        secondN0 = bestN0;
        bestN0.clique = size;
        bestN0.change = n0RemoveChange;
    }
    if (n1RemoveChange > bestN1.change) {
        secondN1 = bestN1;
        bestN1.clique = size;
        bestN1.change = n1RemoveChange;
    }

    // 5. Combine best single moves for n0 and n1
    {
        // 5a. If best relocations go to different cliques
        if (bestN0.clique != bestN1.clique) {
            int combinedChange = bestN0.change + bestN1.change - splitWeight;
            updateBestCombined(bestCombined,
                bestN0.clique,
                bestN1.clique,
                combinedChange);
        }
        // 5b. Both want to move to the same clique
        else {
            // If both want to remove => possibly new clusters (size, size+1)
            if (bestN0.clique == size) {
                int combinedChange = bestN0.change + bestN1.change - splitWeight;
                updateBestCombined(bestCombined,
                    size, size + 1,
                    combinedChange);
            }
            else {
                // There's a conflict -> try bestN0 with secondBestN1
                if (secondN1.change != INT_MIN) {
                    int cChange = bestN0.change + secondN1.change - splitWeight;
                    updateBestCombined(bestCombined,
                        bestN0.clique,
                        secondN1.clique,
                        cChange);
                }
                // And secondBestN0 with bestN1
                if (secondN0.change != INT_MIN) {
                    int cChange = secondN0.change + bestN1.change - splitWeight;
                    updateBestCombined(bestCombined,
                        secondN0.clique,
                        bestN1.clique,
                        cChange);
                }
            }
        }
    }

    // 6. Removing both together
    int removeBoth = n0RemoveChange + n1RemoveChange + mergeWeight - 2 * splitWeight;
    updateBestCombined(bestCombined,
        size, size,
        removeBoth);

    // 7. Final assignments
    // (a) Both
    RelocationBoth.mN0 = bestCombined.mN0;
    RelocationBoth.mC0 = bestCombined.mC0;
    RelocationBoth.mN1 = bestCombined.mN1;
    RelocationBoth.mC1 = bestCombined.mC1;
    RelocationBoth.mChange = bestCombined.mChange;
    RelocationBoth.mProbChange = bestCombined.mChange;
    RelocationBoth.mMoveType = SAMoveType::Both;

    // (b) N0
    RelocationN0.mN0 = bestN0.node;
    RelocationN0.mC0 = bestN0.clique;
    RelocationN0.mChange = bestN0.change;
    RelocationN0.mProbChange = bestN0.change;
    RelocationN0.mMoveType = SAMoveType::N0;

    // (c) N1
    RelocationN1.mN1 = bestN1.node;
    RelocationN1.mC1 = bestN1.clique;
    RelocationN1.mChange = bestN1.change;
    RelocationN1.mProbChange = bestN1.change;
    RelocationN1.mMoveType = SAMoveType::N1;
}


void CPPSolutionBase::SASelectDoubleR2(SARelocation& RelocationBoth, SARelocation& RelocationN0, SARelocation& RelocationN1)
{   
    // move the distribution to the instance
    std::uniform_int_distribution<int> distribution(0, mInstance->getNumberOfEdges() - 1);

    const auto& edge = mInstance->getEdges()[distribution(*mGenerator)];
    // const auto& edge = mInstance->getSampledRandEdge();
    int n0 = edge[0];
    int n1 = edge[1];
    int weight = edge[2];

    RelocationBoth.mN0 = n0;
    RelocationBoth.mN1 = n1;

    int n0Clique = mNodeClique[RelocationBoth.mN0];
    int n1Clique = mNodeClique[RelocationBoth.mN1];

    const int Size = static_cast<int>(mCliqueSizes.size());

    int n0RemoveChange = -mAllConnections[n0Clique][RelocationBoth.mN0];
    int n1RemoveChange = -mAllConnections[n1Clique][RelocationBoth.mN1];

    SASingleRelocation bestN0Relocation;
    bestN0Relocation.node = RelocationBoth.mN0;
    bestN0Relocation.clique = n0Clique;
    bestN0Relocation.change = INT_MIN;
    SASingleRelocation secondBestN0Relocation;
    secondBestN0Relocation.node = RelocationBoth.mN0;
    secondBestN0Relocation.clique = n0Clique;
    secondBestN0Relocation.change = INT_MIN;
    SASingleRelocation bestN1Relocation;
    bestN1Relocation.node = RelocationBoth.mN1;
    bestN1Relocation.clique = n1Clique;
    bestN1Relocation.change = INT_MIN;
    SASingleRelocation secondBestN1Relocation;
    secondBestN1Relocation.node = RelocationBoth.mN1;
    secondBestN1Relocation.clique = n1Clique;
    secondBestN1Relocation.change = INT_MIN;

    SADualSplitRelocation bestCombinedRelocation;
    bestCombinedRelocation.mN0 = RelocationBoth.mN0;
    bestCombinedRelocation.mN1 = RelocationBoth.mN1;
    bestCombinedRelocation.mC0 = n0Clique;
    bestCombinedRelocation.mC1 = n1Clique;
    bestCombinedRelocation.mChange0 = INT_MIN;
    bestCombinedRelocation.mChange1 = INT_MIN;
    bestCombinedRelocation.mChange = INT_MIN;

    if (weight == 0) {
        weight = mInstance->getWeights()[RelocationBoth.mN0][RelocationBoth.mN1];
    }
    int mergeWeight = weight;
    int splitWeight = -weight;

    bool sameClique = n0Clique == n1Clique;
    if (sameClique) {
        mergeWeight = 0;
    }
    else {
        splitWeight = 0;
    }

    long cChange;
    const std::vector<int>& n0CliqueConnections = mAllConnections[n0Clique];
    const std::vector<int>& n1CliqueConnections = mAllConnections[n1Clique];

    for (int c = 0; c < Size; c++) {
        const std::vector<int>& cCliqueConnections = mAllConnections[c];
        int cChange0 = n0RemoveChange + cCliqueConnections[RelocationBoth.mN0];
        int cChange1 = n1RemoveChange + cCliqueConnections[RelocationBoth.mN1];

        if (n0Clique != c && n1Clique != c) {
            // move individually n0 to c
            if (cChange0 > bestN0Relocation.change) {
                secondBestN0Relocation.clique = bestN0Relocation.clique;
                bestN0Relocation.clique = c;
                secondBestN0Relocation.change = bestN0Relocation.change;
                bestN0Relocation.change = cChange0;
            }
            else if (cChange0 > secondBestN0Relocation.change) {
                secondBestN0Relocation.clique = c;
                secondBestN0Relocation.change = cChange0;
            }

            // move individually n1 to c
            if (cChange1 > bestN1Relocation.change) {
                secondBestN1Relocation.clique = bestN1Relocation.clique;
                bestN1Relocation.clique = c;
                secondBestN1Relocation.change = bestN1Relocation.change;
                bestN1Relocation.change = cChange1;
            }
            else if (cChange1 > secondBestN1Relocation.change) {
                secondBestN1Relocation.clique = c;
                secondBestN1Relocation.change = cChange1;
            }

            // move both to c
            cChange = cChange1 + cChange0 + mergeWeight - 2 * splitWeight;
            if (cChange > RelocationBoth.mChange) {
                bestCombinedRelocation.mC0 = c;
                bestCombinedRelocation.mC1 = c;
                bestCombinedRelocation.mChange = cChange;
                if (cChange0 > cChange1) {
                    bestCombinedRelocation.mChange0 = cChange0;
                    bestCombinedRelocation.mChange1 = cChange1 + mergeWeight - 2 * splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    bestCombinedRelocation.mChange0 = cChange0 + mergeWeight - 2 * splitWeight;
                    bestCombinedRelocation.mChange1 = cChange1;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
        }

        if (!sameClique) {
            if (n0Clique != c) {
                int tempChange1 = n1RemoveChange + n0CliqueConnections[RelocationBoth.mN1];
                int mergeAdjustment;

                // slide n1 to n0Clique and n0 to c
                if (c != n1Clique) {
                    mergeAdjustment = -mergeWeight;
                }
                else {
                    mergeAdjustment = -2 * mergeWeight;
                }

                cChange = cChange0 + tempChange1 + mergeAdjustment;

                if (cChange > RelocationBoth.mChange) {
                    bestCombinedRelocation.mC0 = c;
                    bestCombinedRelocation.mC1 = n0Clique;
                    bestCombinedRelocation.mChange = cChange;
                    if (cChange0 > tempChange1) {
                        bestCombinedRelocation.mChange0 = cChange0;
                        bestCombinedRelocation.mChange1 = tempChange1 + mergeAdjustment;
                        bestCombinedRelocation.moveN0First = true;
                    }
                    else {
                        bestCombinedRelocation.mChange0 = cChange0 + mergeAdjustment;
                        bestCombinedRelocation.mChange1 = tempChange1;
                        bestCombinedRelocation.moveN0First = false;
                    }
                }
            }

            if (n1Clique != c) {
                int tempChange0 = n0RemoveChange + n1CliqueConnections[RelocationBoth.mN0];
                int mergeAdjustment;

                // slide n0 to n1Clique and n1 to c
                if (c != n0Clique) {
                    mergeAdjustment = -mergeWeight;
                }
                else {
                    mergeAdjustment = -2 * mergeWeight;
                }

                cChange = cChange1 + tempChange0 + mergeAdjustment;

                if (cChange > RelocationBoth.mChange) {
                    bestCombinedRelocation.mC0 = n1Clique;
                    bestCombinedRelocation.mC1 = c;
                    bestCombinedRelocation.mChange = cChange;
                    if (cChange1 > tempChange0) {
                        bestCombinedRelocation.mChange0 = tempChange0 + mergeAdjustment;
                        bestCombinedRelocation.mChange1 = cChange1;
                        bestCombinedRelocation.moveN0First = false;
                    }
                    else {
                        bestCombinedRelocation.mChange0 = tempChange0;
                        bestCombinedRelocation.mChange1 = cChange1 + mergeAdjustment;
                        bestCombinedRelocation.moveN0First = true;
                    }
                }
            }
        }
    }

    // remove individually
    if (n0RemoveChange > bestN0Relocation.change) {
        secondBestN0Relocation.clique = bestN0Relocation.clique;
        bestN0Relocation.clique = Size;
        secondBestN0Relocation.change = bestN0Relocation.change;
        bestN0Relocation.change = n0RemoveChange;
    }
    if (n1RemoveChange > bestN1Relocation.change) {
        secondBestN1Relocation.clique = bestN1Relocation.clique;
        bestN1Relocation.clique = Size;
        secondBestN1Relocation.change = bestN1Relocation.change;
        bestN1Relocation.change = n1RemoveChange;
    }

    // combine individual moves to together moves
    if (bestN0Relocation.clique != bestN1Relocation.clique) {
        // individual movement of nodes
        cChange = bestN0Relocation.change + bestN1Relocation.change - splitWeight;
        if (cChange > bestCombinedRelocation.mChange) {
            bestCombinedRelocation.mC0 = bestN0Relocation.clique;
            bestCombinedRelocation.mC1 = bestN1Relocation.clique;
            bestCombinedRelocation.mChange = cChange;
            if (bestN0Relocation.change > bestN1Relocation.change) {
                bestCombinedRelocation.mChange0 = bestN0Relocation.change;
                bestCombinedRelocation.mChange1 = bestN1Relocation.change - splitWeight;
                bestCombinedRelocation.moveN0First = true;
            }
            else {
                bestCombinedRelocation.mChange0 = bestN0Relocation.change - splitWeight;
                bestCombinedRelocation.mChange1 = bestN1Relocation.change;
                bestCombinedRelocation.moveN0First = false;
            }
        }
    }
    else {
        // conflict in movement of nodes
        if (bestN0Relocation.clique == Size) {
            // move to newly created cluster
            cChange = bestN0Relocation.change + bestN1Relocation.change - splitWeight;
            if (cChange > bestCombinedRelocation.mChange) {
                bestCombinedRelocation.mChange = cChange;
                if (bestN0Relocation.change > bestN1Relocation.change) {
                    bestCombinedRelocation.mC0 = Size;
                    bestCombinedRelocation.mC1 = Size + 1;
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change - splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    // in this function N0 will always be moved first
                    bestCombinedRelocation.mC0 = Size;
                    bestCombinedRelocation.mC1 = Size + 1;
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change - splitWeight;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
        }
        else {
            cChange = bestN0Relocation.change + secondBestN1Relocation.change - splitWeight;
            if (secondBestN1Relocation.change != INT_MIN && cChange > bestCombinedRelocation.mChange) {
                bestCombinedRelocation.mC0 = bestN0Relocation.clique;
                bestCombinedRelocation.mC1 = secondBestN1Relocation.clique;
                bestCombinedRelocation.mChange = cChange;
                if (bestN0Relocation.change > secondBestN1Relocation.change) {
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change;
                    bestCombinedRelocation.mChange1 = secondBestN1Relocation.change - splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    bestCombinedRelocation.mChange0 = bestN0Relocation.change - splitWeight;
                    bestCombinedRelocation.mChange1 = secondBestN1Relocation.change;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
            cChange = secondBestN0Relocation.change + bestN1Relocation.change - splitWeight;
            if (secondBestN0Relocation.change != INT_MIN && cChange > bestCombinedRelocation.mChange) {
                bestCombinedRelocation.mC0 = secondBestN0Relocation.clique;
                bestCombinedRelocation.mC1 = bestN1Relocation.clique;
                bestCombinedRelocation.mChange = cChange;
                if (secondBestN0Relocation.change > bestN1Relocation.change) {
                    bestCombinedRelocation.mChange0 = secondBestN0Relocation.change;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change - splitWeight;
                    bestCombinedRelocation.moveN0First = true;
                }
                else {
                    bestCombinedRelocation.mChange0 = secondBestN0Relocation.change - splitWeight;
                    bestCombinedRelocation.mChange1 = bestN1Relocation.change;
                    bestCombinedRelocation.moveN0First = false;
                }
            }
        }
    }

    // remove together
    int removeTogether = n0RemoveChange + n1RemoveChange + mergeWeight - 2 * splitWeight;
    if (removeTogether > bestCombinedRelocation.mChange) {
        bestCombinedRelocation.mC0 = Size;
        bestCombinedRelocation.mC1 = Size;
        bestCombinedRelocation.mChange = removeTogether;
        if (n0RemoveChange > n1RemoveChange) {
            bestCombinedRelocation.mChange0 = n0RemoveChange;
            bestCombinedRelocation.mChange1 = n1RemoveChange + mergeWeight - 2 * splitWeight;
            bestCombinedRelocation.moveN0First = true;
        }
        else {
            bestCombinedRelocation.mChange0 = n0RemoveChange + mergeWeight - 2 * splitWeight;
            bestCombinedRelocation.mChange1 = n1RemoveChange;
            bestCombinedRelocation.moveN0First = false;
        }
    }

    RelocationBoth.mN0 = bestCombinedRelocation.mN0;
    RelocationBoth.mC0 = bestCombinedRelocation.mC0;
    RelocationBoth.mN1 = bestCombinedRelocation.mN1;
    RelocationBoth.mC1 = bestCombinedRelocation.mC1;
    RelocationBoth.mChange = bestCombinedRelocation.mChange0 + bestCombinedRelocation.mChange1;
    RelocationBoth.mMoveType = SAMoveType::Both;
    RelocationBoth.mProbChange = RelocationBoth.mChange;

    RelocationN0.mN0 = bestN0Relocation.node;
    RelocationN0.mC0 = bestN0Relocation.clique;
    RelocationN0.mChange = bestN0Relocation.change;
    RelocationN0.mMoveType = SAMoveType::N0;
    RelocationN0.mProbChange = RelocationN0.mChange;

    RelocationN1.mN1 = bestN1Relocation.node;
    RelocationN1.mC1 = bestN1Relocation.clique;
    RelocationN1.mChange = bestN1Relocation.change;
    RelocationN1.mMoveType = SAMoveType::N1;
    RelocationN1.mProbChange = RelocationN1.mChange;

    // test to benefit double moves
    /*
    if (RelocationBoth.mProbChange > 0) {
        RelocationBoth.mProbChange *= 1.1;
    }
    else {
        RelocationBoth.mProbChange *= 0.9;
    }*/
}

void CPPSolutionBase::SASelectR(SARelocation& Relocation)
{
    switch (mSASelectType)
    {
    case SASelectType::Single:
        SASelectSingleR(Relocation);
        break;
    case SASelectType::Dual:
        SASelectDualR(Relocation);
        break;
    case SASelectType::DualNeighbor:
        SASelectDualNeighborOne(Relocation);
        break;
    case SASelectType::SingleEdge:
        SASelectSingleEdge(Relocation, false);
        break;
    case SASelectType::SingleEdgeForcedDual:
        SASelectSingleEdge(Relocation, true);
        break;
    default:
        SASelectDualR(Relocation);
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
    case SAMoveType::None:
        break;
    }

    while (RemoveEmptyCliqueSA(true, true));
}

bool CPPSolutionBase::SimulatedAnnealingWithDoubleMoves(SAParameters& iSAParameters, double& AcceptRelative)
{
    // printf("enter SA");
    nextRelocationCalculated = false;
    int NeiborhoodSize = mInstance->getNumberOfNodes() * getNumberOfCliques() * iSAParameters.neighborhoodFactor;
    int n;
    double Prob;
    double scoreN0;
    double expScoreNone, expScoreBoth, expScoreN0, expScoreN1, expScoreAA;
    double probabilityAA, probabilityAR, probabilityRA;
    double bestSingleAccept, bestAccept;
    int randomValue;
    double sumExpScores;
    double T = 1;
    int BestSol = INT_MIN;
    int cSol = INT_MIN;
    std::vector<int> tNodeClique(mInstance->getNumberOfNodes());
    std::vector<int> tempNodeClique(mInstance->getNumberOfNodes());
    int StartObjective = CalculateObjective();
    int cSolObjective;
    int Accept;
    int AcceptTotal;
    int Stag = 0;
    int counter = 0;
    std::vector<int> NodesChange(mInstance->getNumberOfNodes());
    SARelocation cRelocation, bestRelocation;
    SARelocationStruct cRelocationBoth, cRelocationN0, cRelocationN1;
    int waste = 0;
    removedClique = -1;

    InitAllConnections();
    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());

    T = iSAParameters.mInitTemperature;
    int L = NeiborhoodSize * iSAParameters.mSizeRepeat;

    int counterCool = 0;
    cSolObjective = StartObjective;
    BestSol = StartObjective - 1;
    AcceptTotal = 0;
    int sumNodeChange;

    while (true)
    {

        // gets caught in an endless loop here?
        counter++;
        Accept = 0;
        waste = 0;

        int iterations = L / 2;
        for (int i = 0; i < iterations; i++)
        {
            // clock_t start = clock();
            cRelocationBoth.mChange = INT_MIN;
            cRelocationN0.mChange = INT_MIN;
            cRelocationN1.mChange = INT_MIN;

            SASelectDoubleR(cRelocationBoth, cRelocationN0, cRelocationN1);

            /*
            // adjusted standard
            expScoreNone = 1;
            expScoreBoth = FastExp(cRelocationBoth.mProbChange / T);
            expScoreBoth = expScoreBoth < 1 ? expScoreBoth : 1;
            scoreN0 = cRelocationBoth.mProbChange / 2 > cRelocationN0.mProbChange ? cRelocationBoth.mProbChange / 2 : cRelocationN0.mProbChange;
            expScoreN0 = FastExp(scoreN0 / T);
            expScoreN0 = expScoreN0 < 1 ? expScoreN0 : 1;
            expScoreAA = FastExp((cRelocationBoth.mProbChange - cRelocationN0.mProbChange) / T);
            expScoreAA = expScoreAA < 1 ? expScoreAA : 1;
            expScoreN1 = FastExp(cRelocationN1.mProbChange / T);
            expScoreN1 = expScoreN1 < 1 ? expScoreN1 : 1;
            // sumExpScores = expScoreNone + expScoreBoth + expScoreN0 + expScoreN1;
            //probability0 = expScore0 / sumExpScores;
            probabilityAA = expScoreN0 * expScoreAA;
            probabilityAR = expScoreN0 * (1 - expScoreAA);
            probabilityRA = (1 - expScoreN0) * expScoreN1;
            randomValue = 1 + (*mGenerator)() % 1000;*/

            /*
            grouping double move
            expScoreBoth = FastExp((cRelocationBoth.mProbChange) / T);
            expScoreBoth = expScoreBoth < 1 ? expScoreBoth : 1;
            expScoreN0 = FastExp((cRelocationN0.mProbChange) / T);
            expScoreN0 = expScoreN0 < 1 ? expScoreN0 : 1;
            expScoreN1 = FastExp((cRelocationN1.mProbChange) / T);
            expScoreN1 = expScoreN1 < 1 ? expScoreN1 : 1;
            expScoreNone = ((1 - expScoreN0) + (1 - expScoreN1)) / 2;
            sumExpScores = expScoreN0 + expScoreN1 + expScoreNone;
            probabilityAA = expScoreBoth;
            probabilityAR = (1 - expScoreBoth) * (expScoreN0 / sumExpScores);
            probabilityRA = (1 - expScoreBoth) * (expScoreN1 / sumExpScores);
            randomValue = 1 + (*mGenerator)() % 1000;*/

            /*
            // grouping double move version2
            bestSingleAccept = cRelocationN0.mProbChange > cRelocationN1.mProbChange ? cRelocationN0.mProbChange : cRelocationN1.mProbChange;
            bestAccept = bestSingleAccept > cRelocationBoth.mProbChange ? bestSingleAccept : cRelocationBoth.mProbChange;
            expScoreAA = FastExp(bestAccept / T);
            expScoreAA = expScoreAA < 1 ? expScoreAA : 1;
            expScoreBoth = FastExp((cRelocationBoth.mProbChange - bestSingleAccept) / T);
            expScoreBoth = expScoreBoth < 1 ? expScoreBoth : 1;

            expScoreN0 = FastExp((cRelocationN0.mProbChange) / T);
            expScoreN0 = expScoreN0 < 1 ? expScoreN0 : 1;
            expScoreN1 = FastExp((cRelocationN1.mProbChange) / T);
            expScoreN1 = expScoreN1 < 1 ? expScoreN1 : 1;
            // expScoreNone = ((1 - expScoreN0) + (1 - expScoreN1)) / 2;
            sumExpScores = expScoreN0 + expScoreN1;
            probabilityAA = expScoreAA * expScoreBoth;
            probabilityAR = expScoreAA * (1 - expScoreBoth) * (expScoreN0 / sumExpScores);
            probabilityRA = expScoreAA * (1 - expScoreBoth) * (expScoreN1 / sumExpScores);
            randomValue = 1 + (*mGenerator)() % 1000;*/

            /*
            // grouping double move version3
            bestSingleAccept = cRelocationN0.mProbChange > cRelocationN1.mProbChange ? cRelocationN0.mProbChange : cRelocationN1.mProbChange;
            bestAccept = bestSingleAccept > cRelocationBoth.mProbChange ? bestSingleAccept : cRelocationBoth.mProbChange;
            expScoreAA = FastExp(bestAccept / T);
            expScoreAA = expScoreAA < 1 ? expScoreAA : 1;
            expScoreBoth = FastExp((cRelocationBoth.mProbChange - bestSingleAccept) / T);
            expScoreBoth = expScoreBoth < 1 ? expScoreBoth : 1;

            expScoreN0 = FastExp((cRelocationN0.mProbChange) / T);
            expScoreN1 = FastExp((cRelocationN1.mProbChange) / T);
            expScoreN0 = expScoreN1 >= 1 && expScoreN1 > expScoreN0 ? 0 : expScoreN0;
            expScoreN1 = expScoreN0 >= 1 && expScoreN0 > expScoreN1 ? 0 : expScoreN1;

            sumExpScores = expScoreN0 + expScoreN1;
            probabilityAA = expScoreAA * expScoreBoth;
            probabilityAR = expScoreAA * (1 - expScoreBoth) * (expScoreN0 / sumExpScores);
            probabilityRA = expScoreAA * (1 - expScoreBoth) * (expScoreN1 / sumExpScores);
            randomValue = 1 + (*mGenerator)() % 1000;

            // grouping double move version4 (extension to version 3)
            if (cRelocationBoth.mProbChange > 0 || cRelocationN0.mProbChange > 0 || cRelocationN1.mProbChange > 0) {
                if (cRelocationBoth.mProbChange >= cRelocationN0.mProbChange && cRelocationBoth.mProbChange >= cRelocationN1.mProbChange) {
                    probabilityAA = 1;
                    probabilityAR = 0;
                    probabilityRA = 0;
                }
                else if (cRelocationN0.mProbChange >= cRelocationN1.mProbChange) {
                    probabilityAA = 0;
                    probabilityAR = 1;
                    probabilityRA = 0;
                }
                else {
                    probabilityAA = 0;
                    probabilityAR = 0;
                    probabilityRA = 1;
                }
            }*/

            // grouping double move version5

            bestSingleAccept = cRelocationN0.mProbChange > cRelocationN1.mProbChange ? cRelocationN0.mProbChange : cRelocationN1.mProbChange;
            bestAccept = bestSingleAccept > cRelocationBoth.mProbChange ? bestSingleAccept : cRelocationBoth.mProbChange;
            expScoreAA = std::exp(bestAccept / T);
            expScoreAA = expScoreAA < 1 ? expScoreAA : 1;
            expScoreBoth = std::exp((cRelocationBoth.mProbChange - bestSingleAccept) / T);
            expScoreBoth = expScoreBoth < 1 ? expScoreBoth : 1;

            expScoreN0 = std::exp((cRelocationN0.mProbChange) / T);
            expScoreN1 = std::exp((cRelocationN1.mProbChange) / T);
            expScoreN0 = expScoreN1 >= 1 && expScoreN1 > expScoreN0 ? 0 : expScoreN0;
            expScoreN1 = expScoreN0 >= 1 && expScoreN0 > expScoreN1 ? 0 : expScoreN1;

            sumExpScores = expScoreN0 + expScoreN1;
            probabilityAA = expScoreAA * expScoreBoth;
            probabilityAR = expScoreAA * (1 - expScoreBoth) * (expScoreN0 / sumExpScores);
            probabilityRA = expScoreAA * (1 - expScoreBoth) * (expScoreN1 / sumExpScores);
            randomValue = 1 + (*mGenerator)() % 1000;

            /*
            if (cRelocationBoth.mProbChange > 0 && cRelocationBoth.mProbChange >= cRelocationN0.mProbChange && cRelocationBoth.mProbChange >= cRelocationN1.mProbChange) {
                probabilityAA = 1;
                probabilityAR = 0;
                probabilityRA = 0;
            }*/

            bestRelocation.mChange = INT_MIN;
            if (cRelocationBoth.mProbChange > 0 || cRelocationN0.mProbChange > 0 || cRelocationN1.mProbChange > 0) {
                if (cRelocationBoth.mProbChange >= cRelocationN0.mProbChange && cRelocationBoth.mProbChange >= cRelocationN1.mProbChange) {
                    bestRelocation.mN0 = cRelocationBoth.mN0;
                    bestRelocation.mN1 = cRelocationBoth.mN1;
                    bestRelocation.mC0 = cRelocationBoth.mC0;
                    bestRelocation.mC1 = cRelocationBoth.mC1;
                    bestRelocation.mChange = cRelocationBoth.mChange;
                    bestRelocation.mProbChange = cRelocationBoth.mProbChange;
                    bestRelocation.mMoveType = cRelocationBoth.mMoveType;
                }
                else if (cRelocationN0.mProbChange >= cRelocationN1.mProbChange) {
                    bestRelocation.mN0 = cRelocationN0.mN0;
                    bestRelocation.mN1 = cRelocationN0.mN1;
                    bestRelocation.mC0 = cRelocationN0.mC0;
                    bestRelocation.mC1 = cRelocationN0.mC1;
                    bestRelocation.mChange = cRelocationN0.mChange;
                    bestRelocation.mProbChange = cRelocationN0.mProbChange;
                    bestRelocation.mMoveType = cRelocationN0.mMoveType;
                }
                else {
                    bestRelocation.mN0 = cRelocationN1.mN0;
                    bestRelocation.mN1 = cRelocationN1.mN1;
                    bestRelocation.mC0 = cRelocationN1.mC0;
                    bestRelocation.mC1 = cRelocationN1.mC1;
                    bestRelocation.mChange = cRelocationN1.mChange;
                    bestRelocation.mProbChange = cRelocationN1.mProbChange;
                    bestRelocation.mMoveType = cRelocationN1.mMoveType;
                }
            }

            /*
            * standard double move simulation
            expScoreNone = 1;
            expScoreBoth = FastExp((cRelocationBoth.mProbChange) / T);
            expScoreBoth = expScoreBoth < 1 ? expScoreBoth : 1;
            expScoreN0 = FastExp((cRelocationN0.mProbChange) / T);
            expScoreN0 = expScoreN0 < 1 ? expScoreN0 : 1;
            expScoreN1 = FastExp((cRelocationN1.mProbChange) / T);
            expScoreN1 = expScoreN1 < 1 ? expScoreN1 : 1;
            // sumExpScores = expScoreNone + expScoreBoth + expScoreN0 + expScoreN1;
            //probability0 = expScore0 / sumExpScores;
            probabilityAA = expScoreN0 * expScoreBoth;
            probabilityAR = expScoreN0 * (1 - expScoreBoth);
            probabilityRA = (1 - expScoreN0) * expScoreN1;
            randomValue = 1 + (*mGenerator)() % 1000;
            */

            /*
            * initial test
            SASelectDoubleR(cRelocation1, cRelocation2, cRelocation3);
            expScore0 = FastExp(0);
            expScore1 = FastExp((cRelocation1.mProbChange) / T);
            expScore2 = FastExp((cRelocation2.mProbChange) / T);
            expScore3 = FastExp((cRelocation3.mProbChange) / T);
            sumExpScores = expScore0 + expScore1 + expScore2 + expScore3;
            //probability0 = expScore0 / sumExpScores;
            probability1 = expScore1 / sumExpScores;
            probability2 = expScore2 / sumExpScores;
            probability3 = expScore3 / sumExpScores;
            randomValue = 1 + (*mGenerator)() % 1000;
            */
            // clock_t checkpoint1 = clock();
            // printf("  Prob=%.3f x=%.3f T=%.3f mchange=%d\n", Prob, cRelocation.mChange / T, T, cRelocation.mChange);
            /*if (cRelocation.mChange / T > 900 || cRelocation.mChange / T < -900)
                printf("  Prob=%.3f x=%.3f T=%.3f mchange=%d\n", Prob, cRelocation.mChange / T, T, cRelocation.mChange);*/

            bool skip = true;
            if (probabilityAA * 1000 > randomValue) {
                cRelocation.mN0 = cRelocationBoth.mN0;
                cRelocation.mN1 = cRelocationBoth.mN1;
                cRelocation.mC0 = cRelocationBoth.mC0;
                cRelocation.mC1 = cRelocationBoth.mC1;
                cRelocation.mChange = cRelocationBoth.mChange;
                cRelocation.mProbChange = cRelocationBoth.mProbChange;
                cRelocation.mMoveType = cRelocationBoth.mMoveType;
                skip = false;
            }
            else if ((probabilityAA + probabilityAR) * 1000 > randomValue) {
                cRelocation.mN0 = cRelocationN0.mN0;
                cRelocation.mN1 = cRelocationN0.mN1;
                cRelocation.mC0 = cRelocationN0.mC0;
                cRelocation.mC1 = cRelocationN0.mC1;
                cRelocation.mChange = cRelocationN0.mChange;
                cRelocation.mProbChange = cRelocationN0.mProbChange;
                cRelocation.mMoveType = cRelocationN0.mMoveType;
                skip = false;
            }
            else if ((probabilityAA + probabilityAR + probabilityRA) * 1000 > randomValue) {
                cRelocation.mN0 = cRelocationN1.mN0;
                cRelocation.mN1 = cRelocationN1.mN1;
                cRelocation.mC0 = cRelocationN1.mC0;
                cRelocation.mC1 = cRelocationN1.mC1;
                cRelocation.mChange = cRelocationN1.mChange;
                cRelocation.mProbChange = cRelocationN1.mProbChange;
                cRelocation.mMoveType = cRelocationN1.mMoveType;
                skip = false;
            }

            if (!skip) {
                
                if (cRelocation.mMoveType == SAMoveType::Both) {
                    Accept += 2;
                }
                else {
                    Accept += 1;
                }
                
                // Accept += 1;

                // save missed but better solution; this assumes that it will only be entered if bestRelocation relocates one node and cRelocation relocates two nodes
                if (bestRelocation.mChange > cRelocation.mChange && bestRelocation.mChange + cSolObjective > BestSol) {
                    // std::copy(mNodeClique.begin(), mNodeClique.end(), tempNodeClique.begin());

                    // debug check
                    if (cRelocation.mMoveType != SAMoveType::Both || bestRelocation.mMoveType == SAMoveType::Both)
                        printf("error");

                    /*
                    int prevClique;
                    if (bestRelocation.mMoveType == SAMoveType::N0) {
                        prevClique = mNodeClique[bestRelocation.mN0];
                    }
                    else if (bestRelocation.mMoveType == SAMoveType::N1) {
                        prevClique = mNodeClique[bestRelocation.mN1];
                    }*/
                    
                    ApplyRelocation(bestRelocation);
                    cSolObjective += bestRelocation.mChange;
                    if (removedClique > -1) {
                        if (cRelocation.mC0 > removedClique) cRelocation.mC0 -= 1;
                        if (cRelocation.mC1 > removedClique) cRelocation.mC1 -= 1;
                        
                        int newClique = mAllConnections.size();
                        if (cRelocation.mC0 == removedClique) cRelocation.mC0 = newClique;
                        if (cRelocation.mC1 == removedClique) cRelocation.mC1 = newClique;
                    }

                    if (BestSol < cSolObjective)
                    {
                        BestSol = cSolObjective;
                        std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());
                    }

                    cSolObjective -= bestRelocation.mChange;
                    // CreateFromNodeClique(tempNodeClique);
                    // InitAllConnections();
                    removedClique = -1;
                }

                ApplyRelocation(cRelocation);
                cSolObjective += cRelocation.mChange;
                /*
                if (cSol != cSolObjective)
                    cSol = cSolObjective;*/

                if (BestSol < cSolObjective)
                {
                    BestSol = cSolObjective;
                    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());
                }
            }

            /*
            if (nextRelocationCalculated) {
                SASelectR(cRelocation);
                Prob = FastExp(cRelocation.mChange / T);
                if (Prob * 1000 > (*mGenerator)() % 1000)
                {
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
            }*/
            // printf("%d ; %d\n", checkpoint1 - start, clock() - checkpoint1);
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

        if (static_cast<double>(Accept) / L < iSAParameters.mMinAccept)
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
    return true;
}

bool CPPSolutionBase::SimulatedAnnealingCool(SAParameters& iSAParameters, double& AcceptRelative)
{
    nextRelocationCalculated = false;
    int NeiborhoodSize = mInstance->getNumberOfNodes() * getNumberOfCliques() * iSAParameters.neighborhoodFactor;
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
    int L = NeiborhoodSize * iSAParameters.mSizeRepeat;

    int counterCool = 0;
    cSolObjective = StartObjective;
    AcceptTotal = 0;
    int sumNodeChange;

    cooldown_period = 0.3 * mInstance->getNumberOfNodes();
    eligible_nodes.reserve(mInstance->getNumberOfNodes());

    eligible_nodes.clear();
    for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
        eligible_nodes.push_back(i);
    }
    cooldown_buffer = std::vector<int>(cooldown_period, -1);

    int iteration = 0;
    while (true)
    {
        counter++;
        Accept = 0;
        waste = 0;

        for (int i = 0; i < L; i++)
        {   
            int buffer_index = iteration % cooldown_period;
            int cool_node = cooldown_buffer[buffer_index];
            if (cool_node != -1)
                eligible_nodes.push_back(cool_node);

            int rand_i = (*mGenerator)() % eligible_nodes.size();

            cRelocation.mN1 = cRelocation.mN0;
            cRelocation.mN0 = eligible_nodes[rand_i];
            eligible_nodes[rand_i] = eligible_nodes.back();
            eligible_nodes.pop_back();

            cooldown_buffer[buffer_index] = cRelocation.mN0;

            cRelocation.mChange = INT_MIN;
            SASelectDual(cRelocation);

            cRelocation.mProbChange = cRelocation.mChange;

            int numMoves = 1;
            if (cRelocation.mMoveType != SAMoveType::N0 && cRelocation.mMoveType != SAMoveType::N1) {
                numMoves = 1;
            }
            if (cRelocation.forceAccept || FastExp(cRelocation.mProbChange / (T * numMoves)) * 1000 > 1 + (*mGenerator)() % 1000)
            {
                prevAccepted = true;
                Accept += numMoves;
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
            else {
                prevAccepted = false;
            }

            iteration++;
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

        if (static_cast<double>(Accept) / L < iSAParameters.mMinAccept)
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
    return true;
}

bool CPPSolutionBase::SimulatedAnnealing(SAParameters& iSAParameters, double& AcceptRelative)
{   
    // printf("enter SA");
    nextRelocationCalculated = false;
    int NeiborhoodSize = mInstance->getNumberOfNodes() * getNumberOfCliques() * iSAParameters.neighborhoodFactor;
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
    int L = NeiborhoodSize * iSAParameters.mSizeRepeat;

    int counterCool = 0;
    cSolObjective = StartObjective;
    AcceptTotal = 0;
    int sumNodeChange;

    int numRelocations = 0;

    while (true)
    {   
        
        // gets caught in an endless loop here?
        counter++;
        Accept = 0;
        waste = 0;

        for (int i = 0; i < L; i++)
        {
            // clock_t start = clock();
            cRelocation.mChange = INT_MIN;
            SASelectR(cRelocation);

            int numMoves = 1;
            if (cRelocation.mMoveType != SAMoveType::N0 && cRelocation.mMoveType != SAMoveType::N1) {
                numMoves = 1;
            }

            // Prob = FastExp(cRelocation.mProbChange / (T * numMoves));
            // clock_t checkpoint1 = clock();
            // printf("  Prob=%.3f x=%.3f T=%.3f mchange=%d\n", Prob, cRelocation.mChange / T, T, cRelocation.mChange);
            /*if (cRelocation.mChange / T > 900 || cRelocation.mChange / T < -900)
                printf("  Prob=%.3f x=%.3f T=%.3f mchange=%d\n", Prob, cRelocation.mChange / T, T, cRelocation.mChange);*/
            if (cRelocation.forceAccept || FastExp(cRelocation.mProbChange / (T * numMoves)) * 1000 > 1 + (*mGenerator)() % 1000)
            {   
                prevAccepted = true;
                Accept += numMoves;
                ApplyRelocation(cRelocation);
                numRelocations++;
                cSolObjective += cRelocation.mChange;
                if (cSol != cSolObjective)
                    cSol = cSolObjective;

                if (BestSol < cSolObjective)
                {
                    BestSol = cSolObjective;
                    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());
                }
            } else {
                prevAccepted = false;
            }
            /*
            if (nextRelocationCalculated) {
                SASelectR(cRelocation);
                Prob = FastExp(cRelocation.mChange / T);
                if (Prob * 1000 > (*mGenerator)() % 1000)
                {
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
            }*/
            // printf("%d ; %d\n", checkpoint1 - start, clock() - checkpoint1);
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

        if (static_cast<double>(Accept) / L < iSAParameters.mMinAccept)
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

    printf("numRelocations=%d\n", numRelocations);
    return true;
}

bool CPPSolutionBase::CalibrateSADoubleMoves(SAParameters& iSAParameters, double& Accept)
{
    nextRelocationCalculated = false;
    int MaxStep = (mInstance->getNumberOfNodes() * getNumberOfCliques() * iSAParameters.mSizeRepeat * iSAParameters.neighborhoodFactor) / 2;
    int n;
    double Prob;
    double scoreN0;
    double bestSingleAccept, bestAccept;
    double expScoreNone, expScoreBoth, expScoreN0, expScoreN1, expScoreAA;
    double probabilityAA, probabilityAR, probabilityRA;
    int randomValue;
    double sumExpScores;
    double T = 1;
    int BestSol = INT_MIN;
    int cSol = INT_MIN;
    std::vector<int> tNodeClique(mInstance->getNumberOfNodes());
    std::vector<int> tempNodeClique(mInstance->getNumberOfNodes());
    int StartObjective = CalculateObjective();
    
    int cSolObjective;
    int NoImprove = 0;
    Accept = 0;
    SARelocation cRelocation, bestRelocation;
    SARelocationStruct cRelocationBoth, cRelocationN0, cRelocationN1;

    InitAllConnections();
    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());

    BestSol = StartObjective - 1;
    cSolObjective = StartObjective;
    for (int i = 0; i < MaxStep; i++)
    {
        NoImprove++;

        cRelocationBoth.mChange = INT_MIN;
        cRelocationN0.mChange = INT_MIN;
        cRelocationN1.mChange = INT_MIN;
        T = iSAParameters.mInitTemperature;

        SASelectDoubleR(cRelocationBoth, cRelocationN0, cRelocationN1);
        
        // grouping double move version5
        bestSingleAccept = cRelocationN0.mProbChange > cRelocationN1.mProbChange ? cRelocationN0.mProbChange : cRelocationN1.mProbChange;
        bestAccept = bestSingleAccept > cRelocationBoth.mProbChange ? bestSingleAccept : cRelocationBoth.mProbChange;
        expScoreAA = std::exp(bestAccept / T); // why the fuck is this not returning what it should?
        expScoreAA = expScoreAA < 1 ? expScoreAA : 1;
        expScoreBoth = std::exp((cRelocationBoth.mProbChange - bestSingleAccept) / T);
        expScoreBoth = expScoreBoth < 1 ? expScoreBoth : 1;

        expScoreN0 = std::exp((cRelocationN0.mProbChange) / T);
        expScoreN1 = std::exp((cRelocationN1.mProbChange) / T);
        expScoreN0 = expScoreN1 >= 1 && expScoreN1 > expScoreN0 ? 0 : expScoreN0;
        expScoreN1 = expScoreN0 >= 1 && expScoreN0 > expScoreN1 ? 0 : expScoreN1;

        sumExpScores = expScoreN0 + expScoreN1;
        probabilityAA = expScoreAA * expScoreBoth;
        probabilityAR = expScoreAA * (1 - expScoreBoth) * (expScoreN0 / sumExpScores);
        probabilityRA = expScoreAA * (1 - expScoreBoth) * (expScoreN1 / sumExpScores);
        randomValue = 1 + (*mGenerator)() % 1000;

        /*
        if (cRelocationBoth.mProbChange > 0 && cRelocationBoth.mProbChange >= cRelocationN0.mProbChange && cRelocationBoth.mProbChange >= cRelocationN1.mProbChange) {
            probabilityAA = 1;
            probabilityAR = 0;
            probabilityRA = 0;
        }*/

        bestRelocation.mChange = INT_MIN;
        if (cRelocationBoth.mProbChange > 0 || cRelocationN0.mProbChange > 0 || cRelocationN1.mProbChange > 0) {
            if (cRelocationBoth.mProbChange >= cRelocationN0.mProbChange && cRelocationBoth.mProbChange >= cRelocationN1.mProbChange) {
                bestRelocation.mN0 = cRelocationBoth.mN0;
                bestRelocation.mN1 = cRelocationBoth.mN1;
                bestRelocation.mC0 = cRelocationBoth.mC0;
                bestRelocation.mC1 = cRelocationBoth.mC1;
                bestRelocation.mChange = cRelocationBoth.mChange;
                bestRelocation.mProbChange = cRelocationBoth.mProbChange;
                bestRelocation.mMoveType = cRelocationBoth.mMoveType;
            }
            else if (cRelocationN0.mProbChange >= cRelocationN1.mProbChange) {
                bestRelocation.mN0 = cRelocationN0.mN0;
                bestRelocation.mN1 = cRelocationN0.mN1;
                bestRelocation.mC0 = cRelocationN0.mC0;
                bestRelocation.mC1 = cRelocationN0.mC1;
                bestRelocation.mChange = cRelocationN0.mChange;
                bestRelocation.mProbChange = cRelocationN0.mProbChange;
                bestRelocation.mMoveType = cRelocationN0.mMoveType;
            }
            else {
                bestRelocation.mN0 = cRelocationN1.mN0;
                bestRelocation.mN1 = cRelocationN1.mN1;
                bestRelocation.mC0 = cRelocationN1.mC0;
                bestRelocation.mC1 = cRelocationN1.mC1;
                bestRelocation.mChange = cRelocationN1.mChange;
                bestRelocation.mProbChange = cRelocationN1.mProbChange;
                bestRelocation.mMoveType = cRelocationN1.mMoveType;
            }
        }


        bool skip = true;
        if (probabilityAA * 1000 > randomValue) {
            cRelocation.mN0 = cRelocationBoth.mN0;
            cRelocation.mN1 = cRelocationBoth.mN1;
            cRelocation.mC0 = cRelocationBoth.mC0;
            cRelocation.mC1 = cRelocationBoth.mC1;
            cRelocation.mChange = cRelocationBoth.mChange;
            cRelocation.mProbChange = cRelocationBoth.mProbChange;
            cRelocation.mMoveType = cRelocationBoth.mMoveType;
            skip = false;
        }
        else if ((probabilityAA + probabilityAR) * 1000 > randomValue) {
            cRelocation.mN0 = cRelocationN0.mN0;
            cRelocation.mN1 = cRelocationN0.mN1;
            cRelocation.mC0 = cRelocationN0.mC0;
            cRelocation.mC1 = cRelocationN0.mC1;
            cRelocation.mChange = cRelocationN0.mChange;
            cRelocation.mProbChange = cRelocationN0.mProbChange;
            cRelocation.mMoveType = cRelocationN0.mMoveType;
            skip = false;
        }
        else if ((probabilityAA + probabilityAR + probabilityRA) * 1000 > randomValue) {
            cRelocation.mN0 = cRelocationN1.mN0;
            cRelocation.mN1 = cRelocationN1.mN1;
            cRelocation.mC0 = cRelocationN1.mC0;
            cRelocation.mC1 = cRelocationN1.mC1;
            cRelocation.mChange = cRelocationN1.mChange;
            cRelocation.mProbChange = cRelocationN1.mProbChange;
            cRelocation.mMoveType = cRelocationN1.mMoveType;
            skip = false;
        }

        if (!skip) {
            if (cRelocation.mMoveType == SAMoveType::Both) {
                Accept += 1;
            }
            else {
                Accept += 0.5;
            }

            //Accept += 1;

            // save missed but better solution; this assumes that it will only be entered if bestRelocation relocates one node and cRelocation relocates two nodes
            if (bestRelocation.mChange > cRelocation.mChange && bestRelocation.mChange + cSolObjective > BestSol) {
                std::copy(mNodeClique.begin(), mNodeClique.end(), tempNodeClique.begin());

                // debug check
                if (cRelocation.mMoveType != SAMoveType::Both || bestRelocation.mMoveType == SAMoveType::Both)
                    printf("error");

                /*
                int prevClique;
                if (bestRelocation.mMoveType == SAMoveType::N0) {
                    prevClique = mNodeClique[bestRelocation.mN0];
                }
                else if (bestRelocation.mMoveType == SAMoveType::N1) {
                    prevClique = mNodeClique[bestRelocation.mN1];
                }*/

                ApplyRelocation(bestRelocation);
                cSolObjective += bestRelocation.mChange;
                if (removedClique > -1) {
                    if (cRelocation.mC0 > removedClique) cRelocation.mC0--;
                    if (cRelocation.mC1 > removedClique) cRelocation.mC1--;
                }

                if (BestSol < cSolObjective)
                {
                    BestSol = cSolObjective;
                    std::copy(mNodeClique.begin(), mNodeClique.end(), tNodeClique.begin());
                }

                cSolObjective -= bestRelocation.mChange;
                // CreateFromNodeClique(tempNodeClique);
                // InitAllConnections();
                removedClique = -1;
            }

            ApplyRelocation(cRelocation);

            cSolObjective += cRelocation.mChange;
            /*
            if (cSol != cSolObjective)
                cSol = cSolObjective;*/

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

bool CPPSolutionBase::CalibrateSACool(SAParameters& iSAParameters, double& Accept)
{
    nextRelocationCalculated = false;
    int MaxStep = mInstance->getNumberOfNodes() * getNumberOfCliques() * iSAParameters.mSizeRepeat * iSAParameters.neighborhoodFactor;
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

    cooldown_period = 0.3 * mInstance->getNumberOfNodes();
    eligible_nodes.reserve(mInstance->getNumberOfNodes());

    eligible_nodes.clear();
    for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
        eligible_nodes.push_back(i);
    }
    cooldown_buffer = std::vector<int>(cooldown_period, -1);

    cSolObjective = StartObjective;
    int iteration = 0;
    for (int i = 0; i < MaxStep; i++)
    {
        NoImprove++;

        int buffer_index = iteration % cooldown_period;
        int cool_node = cooldown_buffer[buffer_index];
        if (cool_node != -1)
            eligible_nodes.push_back(cool_node);

        int rand_i = (*mGenerator)() % eligible_nodes.size();

        Relocation.mN1 = Relocation.mN0;
        Relocation.mN0 = eligible_nodes[rand_i];
        eligible_nodes[rand_i] = eligible_nodes.back();
        eligible_nodes.pop_back();

        cooldown_buffer[buffer_index] = Relocation.mN0;

        Relocation.mChange = INT_MIN;
        SASelectDual(Relocation);

        Relocation.mProbChange = Relocation.mChange;

        int numMoves = 1;
        if (Relocation.mMoveType != SAMoveType::N0 && Relocation.mMoveType != SAMoveType::N1) {
            numMoves = 1;
        }
        T = iSAParameters.mInitTemperature;

        if (Relocation.forceAccept || std::exp(Relocation.mProbChange / (T * numMoves)) * 1000 > 1 + (*mGenerator)() % 1000)
        {
            prevAccepted = true;
            Accept += numMoves;

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
        else {
            prevAccepted = false;
        }
        iteration++;
    }

    CreateFromNodeClique(tNodeClique);

    Accept = Accept / MaxStep;
    return true;
}

bool CPPSolutionBase::CalibrateSA(SAParameters& iSAParameters, double& Accept)
{   
    nextRelocationCalculated = false;
    int MaxStep = mInstance->getNumberOfNodes() * getNumberOfCliques() * iSAParameters.mSizeRepeat * iSAParameters.neighborhoodFactor;
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
        SASelectR(Relocation);
        int numMoves = 1;
        if (Relocation.mMoveType != SAMoveType::N0 && Relocation.mMoveType != SAMoveType::N1) {
            numMoves = 1;
        }
        T = iSAParameters.mInitTemperature;

        //Prob = std::exp(Relocation.mProbChange / (T * numMoves));

        if (Relocation.forceAccept || std::exp(Relocation.mProbChange / (T * numMoves)) * 1000 > 1 + (*mGenerator)() % 1000)
        {   
            prevAccepted = true;
            Accept += numMoves;

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
        } else {
            prevAccepted = false;
        }
        /*
        if (nextRelocationCalculated) {
            SASelectR(Relocation);
            T = iSAParameters.mInitTemperature;
            Prob = std::exp(Relocation.mChange / T);
            if (Prob * 1000 > (*mGenerator)() % 1000)
            {
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
        }*/
    }

    CreateFromNodeClique(tNodeClique);

    Accept = Accept / MaxStep;
    return true;
}
/*
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
*/
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
                mAllConnections.erase(mAllConnections.begin() + i);
            }

            removedClique = i;
            if (nextAcceptRelocation.mMoveType == SAMoveType::N0) {
                if (nextAcceptRelocation.mC0 == static_cast<int>(i)) {
                    nextAcceptRelocation.mC0 = static_cast<int>(mCliqueSizes.size());
                }
                else if (nextAcceptRelocation.mC0 > static_cast<int>(i)) {
                    nextAcceptRelocation.mC0--;
                }
            }
            else if (nextAcceptRelocation.mMoveType == SAMoveType::N1) {
                if (nextAcceptRelocation.mC1 == static_cast<int>(i)) {
                    nextAcceptRelocation.mC1 = static_cast<int>(mCliqueSizes.size());
                }
                else if (nextAcceptRelocation.mC1 > static_cast<int>(i)) {
                    nextAcceptRelocation.mC1--;
                }
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

