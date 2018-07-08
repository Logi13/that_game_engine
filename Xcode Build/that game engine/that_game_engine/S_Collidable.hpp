#ifndef S_Collidable_hpp
#define S_Collidable_hpp

#include <vector>
#include <memory>
#include <set>

#include "Object.hpp"
#include "QuadTree.hpp"
#include "Bitmask.hpp"

class S_Collidable
{
public:
    S_Collidable();
    
    void Add(std::vector<std::shared_ptr<Object>>& objects);
    void ProcessRemovals();
    
    void Resolve();
    
private:
    void ProcessCollisions(std::vector<std::shared_ptr<Object>>& first, std::vector<std::shared_ptr<Object>>& second);
    
    std::map<CollisionLayer, Bitmask> collisionLayers;
    std::map<CollisionLayer, std::vector<std::shared_ptr<C_Collider>>> collidables;
    
    QuadTree collisionTree;
};



#endif /* S_Collidable_hpp */
