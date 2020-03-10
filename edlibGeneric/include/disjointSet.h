//
// Created by mobin on 2/28/20.
//
#include <vector>
#include <unordered_map>
using namespace std;

#ifndef EDLIBGENERIC_PYTHON_DISJOINTSET_H
#define EDLIBGENERIC_PYTHON_DISJOINTSET_H

/**
 * A class for mapping equal symbols to one of those symbols using
 * disjoint sets
 * Each symbol is considered as a set with one element at first
 * Then by adding equal symbols, the set expands
 */
template <class AlphabetType>
class DisjointSet{
private:
    // parent is a hash map which has symbols as keys
    // and their parents as values
    unordered_map<AlphabetType, AlphabetType> parent;
    // rank is a hash map which as symbols as keys
    // and their parents as values
    unordered_map<AlphabetType, int> rank;
public:
    /**
     * add a symbol to the disjoint-set structure
     * don't add if it already exists
     * @param symbol
     */
    void add(AlphabetType symbol){
        if (parent.find(symbol) == parent.end()) {
            parent[symbol] = symbol;
            rank[symbol] = 0;
        }
    }
    /**
     * find the root parent of the given symbol
     * because that represents the whole set
     * connect the root parent to the symbol to make its actual parent
     * this direct connection makes later searches faster
     * @param symbol
     * @return the root parent
     */
    AlphabetType find(AlphabetType symbol){
        if (parent.find(symbol) == parent.end()) {
            return NULL;
        }else {
            if (parent[symbol] != symbol)
                parent[symbol] = find(parent[symbol]);
            return parent[symbol];
        }
    }
    /**
     * merge the root parents of the given symbols
     * @param symbol1
     * @param symbol2
     */
    void merge(AlphabetType symbol1, AlphabetType symbol2){
        AlphabetType root1 = find(symbol1);
        AlphabetType root2 = find(symbol2);
        if (root1 == root2)
            return;
        if (rank[root1] < rank[root2]) {
            parent[root1] = root2;
        }
        else if (rank[root1] > rank[root2]) {
            parent[root2] = root1;
        }
        else {
            parent[root2] = root1;
            rank[root1] = rank[root1] + 1;
        }
    }
};

#endif //EDLIBGENERIC_PYTHON_DISJOINTSET_H
