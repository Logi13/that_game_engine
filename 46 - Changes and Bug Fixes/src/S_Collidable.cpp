#include "S_Collidable.hpp"

S_Collidable::S_Collidable(Quadtree& collisionTree) : collisionTree(collisionTree)
{
    Bitmask defaultCollisions;
    defaultCollisions.SetBit((int)CollisionLayer::Default);
    collisionLayers.insert(std::make_pair(CollisionLayer::Default, defaultCollisions));
    
    collisionLayers.insert(std::make_pair(CollisionLayer::Tile, Bitmask(0))); 
    
    Bitmask playerCollisions;
    playerCollisions.SetBit((int) CollisionLayer::Default);
    playerCollisions.SetBit((int) CollisionLayer::Tile);
    playerCollisions.SetBit((int) CollisionLayer::NPC);
    collisionLayers.insert(std::make_pair(CollisionLayer::Player, playerCollisions));
    
    Bitmask projectileCollisions;
    projectileCollisions.SetBit((int) CollisionLayer::Tile);
    projectileCollisions.SetBit((int) CollisionLayer::NPC);
    collisionLayers.insert(std::make_pair(CollisionLayer::Projectile, projectileCollisions));
}

void S_Collidable::Add(std::vector<std::shared_ptr<Object>>& objects)
{
    for (auto o : objects)
    {
        auto collider = o->GetComponent<C_BoxCollider>();
        if (collider)
        {
            CollisionLayer layer = collider->GetLayer();
            
            auto itr = collidables.find(layer);
            
            if (itr != collidables.end())
            {
                collidables[layer].push_back(collider);
            }
            else
            {
                collidables.insert(std::make_pair(layer,  std::vector<std::shared_ptr<C_BoxCollider>>{collider}));
            }
        }
    }
}

void S_Collidable::ProcessRemovals()
{
    for (auto& layer : collidables)
    {
        auto itr = layer.second.begin();
        while (itr != layer.second.end())
        {
            if ((*itr)->owner->IsQueuedForRemoval())
            {
                itr = layer.second.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
    }
}

void S_Collidable::UpdatePositions(std::vector<std::shared_ptr<Object>>& objects)
{
    for (auto o : objects)
    {
        if(!o->transform->isStatic())
        {
            auto collider = o->GetComponent<C_BoxCollider>();
            
            if(collider)
            {
                collisionTree.UpdatePosition(collider);
            }
        }
    }
}

void S_Collidable::Resolve()
{
    for (auto maps = collidables.begin(); maps != collidables.end(); ++maps)
    {
        // If this layer collides with nothing then no need to perform any furter checks.
        if(collisionLayers[maps->first].GetMask() == 0)
        {
            continue;
        }
        
        for (auto collidable : maps->second)
        {
            // If this collidable is static then no need to check if its colliding with other objects.
            if (collidable->owner->transform->isStatic())
            {
                continue;
            }
            
            std::vector<std::shared_ptr<C_BoxCollider>> collisions = collisionTree.Search(collidable->GetCollidable());
            
            for (auto collision : collisions)
            {
                // Make sure we do not resolve collisions between the the same object.
                if (collidable->owner->instanceID->Get() == collision->owner->instanceID->Get())
                {
                    continue;
                }
                
                bool layersCollide = collisionLayers[collidable->GetLayer()].GetBit(((int)collision->GetLayer()));
                
                if(layersCollide)
                {
                    Manifold m = collidable->Intersects(collision);
                    
                    if(m.colliding)
                    {
                        auto collisionPair = objectsColliding.emplace(std::make_pair(collidable, collision));
                        
                        if(collisionPair.second)
                        {
                            collidable->owner->OnCollisionEnter(collision);
                            collision->owner->OnCollisionEnter(collidable);
                        }
                    
                        Debug::DrawRect(collision->GetCollidable(), sf::Color::Red);
                        Debug::DrawRect(collidable->GetCollidable(), sf::Color::Red);
                        
                        if(collision->owner->transform->isStatic())
                        {
                            collidable->ResolveOverlap(m);
                        }
                        else
                        {
                            
                            //TODO: how shall we handle collisions when both objects are not static?
                            // We could implement rigidbodies and mass.
                            collidable->ResolveOverlap(m);
                        }
                        
                    }
                }
            }
        }
    }
}

void S_Collidable::Update()
{
    collisionTree.DrawDebug();
    
    ProcessCollidingObjects();
    
    collisionTree.Clear();
    for (auto maps = collidables.begin(); maps != collidables.end(); ++maps)
    {
        for (auto collidable : maps->second)
        {
            collisionTree.Insert(collidable);
        }
    }
    
    Resolve();
}

void S_Collidable::ProcessCollidingObjects()
{
    auto itr = objectsColliding.begin();
    while (itr != objectsColliding.end())
    {
        auto pair = *itr;
        
        std::shared_ptr<C_BoxCollider> first = pair.first;
        std::shared_ptr<C_BoxCollider> second = pair.second;
        
        if(first->owner->IsQueuedForRemoval() || second->owner->IsQueuedForRemoval())
        {
            first->owner->OnCollisionExit(second);
            second->owner->OnCollisionExit(first);
            
            itr = objectsColliding.erase(itr);

        }
        else
        {
            Manifold m = first->Intersects(second);
            
            if (!m.colliding)
            {
                first->owner->OnCollisionExit(second);
                second->owner->OnCollisionExit(first);
                
                itr = objectsColliding.erase(itr);
            }
            else
            {
                first->owner->OnCollisionStay(second);
                second->owner->OnCollisionStay(first);
                
                ++itr;
            }
        }
    
    }
}



