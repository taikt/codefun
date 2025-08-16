/*
 * Circular Reference and Complex Memory Leaks
 * Memory leaks phức tạp: circular references, containers
 */

#include <iostream>
#include <vector>
#include <map>
#include <memory>

// Forward declaration
class Parent;
class Child;

class Child {
public:
    int id;
    Parent* parent;  // Raw pointer to parent
    std::vector<int*> data;  // Vector of raw pointers
    
    Child(int childId) : id(childId), parent(nullptr) {
        // Allocate some data
        for (int i = 0; i < 10; i++) {
            data.push_back(new int(i * childId));
        }
    }
    
    ~Child() {
        std::cout << "Child " << id << " destructor called" << std::endl;
        // MEMORY LEAK: data vector elements not cleaned!
        // Should delete all int* in data vector
    }
    
    void setParent(Parent* p) {
        parent = p;
    }
};

class Parent {
public:
    int id;
    std::vector<Child*> children;  // Raw pointers to children
    std::map<int, double*> calculations;  // Map with raw pointer values
    
    Parent(int parentId) : id(parentId) {}
    
    ~Parent() {
        std::cout << "Parent " << id << " destructor called" << std::endl;
        // MEMORY LEAK: children not deleted!
        // MEMORY LEAK: calculations map values not deleted!
    }
    
    void addChild(Child* child) {
        children.push_back(child);
        child->setParent(this);  // Creates circular reference
        
        // Add some calculation data
        calculations[child->id] = new double(child->id * 3.14);
    }
    
    void removeChild(int childId) {
        // MEMORY LEAK: Child object not deleted when removed!
        for (auto it = children.begin(); it != children.end(); ++it) {
            if ((*it)->id == childId) {
                children.erase(it);
                break;
            }
        }
        
        // MEMORY LEAK: calculation data for this child not deleted!
        if (calculations.find(childId) != calculations.end()) {
            calculations.erase(childId);  // Deletes map entry but not the double*
        }
    }
};

class GraphNode {
public:
    int value;
    std::vector<GraphNode*> neighbors;  // Raw pointers
    std::map<GraphNode*, int*> edgeWeights;  // Map with raw pointer values
    
    GraphNode(int val) : value(val) {}
    
    ~GraphNode() {
        std::cout << "GraphNode " << value << " destructor" << std::endl;
        // MEMORY LEAK: edgeWeights values not cleaned!
        // MEMORY LEAK: No cleanup of neighbor relationships
    }
    
    void addNeighbor(GraphNode* neighbor, int weight) {
        neighbors.push_back(neighbor);
        neighbor->neighbors.push_back(this);  // Bidirectional reference
        
        edgeWeights[neighbor] = new int(weight);
        neighbor->edgeWeights[this] = new int(weight);
    }
};

void createCircularReferences() {
    Parent* parent1 = new Parent(1);
    Parent* parent2 = new Parent(2);
    
    // Create children with circular references
    for (int i = 0; i < 5; i++) {
        Child* child = new Child(i);
        parent1->addChild(child);
    }
    
    for (int i = 5; i < 10; i++) {
        Child* child = new Child(i);
        parent2->addChild(child);
    }
    
    // Cross-reference children (more complex circular refs)
    if (!parent1->children.empty() && !parent2->children.empty()) {
        parent1->children[0]->setParent(parent2);  // Child has 2 parents now
        parent2->children[0]->setParent(parent1);  // Cross reference
    }
    
    // MEMORY LEAK: parent1 and parent2 never deleted!
    // Even if deleted, their destructors don't clean up properly
    
    // Try to remove some children (creates more leaks)
    parent1->removeChild(0);
    parent2->removeChild(5);
}

void createGraphLeaks() {
    std::vector<GraphNode*> graph;
    
    // Create nodes
    for (int i = 0; i < 10; i++) {
        graph.push_back(new GraphNode(i));
    }
    
    // Create complex interconnections
    for (int i = 0; i < 10; i++) {
        for (int j = i + 1; j < 10; j++) {
            if ((i + j) % 3 == 0) {  // Some arbitrary condition
                graph[i]->addNeighbor(graph[j], i * j);
            }
        }
    }
    
    // MEMORY LEAK: graph vector nodes never deleted!
    // MEMORY LEAK: All edgeWeights allocations leak!
    
    // Clear vector but don't delete nodes
    graph.clear();  // Pointers lost forever!
}

class ContainerWithRawPointers {
    std::vector<int*> intPtrs;
    std::map<std::string, double*> doublePtrs;
    std::vector<std::vector<char*>> nestedPtrs;
    
public:
    ContainerWithRawPointers() {
        // Fill containers with raw pointers
        for (int i = 0; i < 20; i++) {
            intPtrs.push_back(new int(i));
            doublePtrs[std::to_string(i)] = new double(i * 1.5);
        }
        
        // Create nested structure
        for (int i = 0; i < 5; i++) {
            std::vector<char*> innerVec;
            for (int j = 0; j < 4; j++) {
                char* str = new char[50];
                sprintf(str, "String_%d_%d", i, j);
                innerVec.push_back(str);
            }
            nestedPtrs.push_back(innerVec);
        }
    }
    
    ~ContainerWithRawPointers() {
        // MEMORY LEAK: None of the allocated memory is freed!
        // Should iterate through all containers and delete elements
        std::cout << "ContainerWithRawPointers destructor (leaky!)" << std::endl;
    }
    
    void addMoreData() {
        // Add more raw pointers without cleaning old ones
        for (int i = 100; i < 120; i++) {
            intPtrs.push_back(new int(i));
        }
        
        // MEMORY LEAK: Growing containers with no cleanup
    }
};

int main() {
    std::cout << "Testing circular references and complex leaks..." << std::endl;
    
    createCircularReferences();
    createGraphLeaks();
    
    ContainerWithRawPointers* container = new ContainerWithRawPointers();
    container->addMoreData();
    // MEMORY LEAK: container never deleted!
    
    std::cout << "Complex leaks test completed (with many leaks!)" << std::endl;
    return 0;
}
