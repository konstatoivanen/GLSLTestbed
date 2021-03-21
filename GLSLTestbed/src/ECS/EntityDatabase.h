#pragma once
#include "Core/IService.h"
#include "Core/BufferView.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Material.h"
#include <hlslmath.h>

namespace PK::ECS
{
    using namespace PK::Math;
    using namespace PK::Utilities;
    using namespace PK::Core;

    enum class ENTITY_GROUPS
    {
        INVALID = 0,
        INACTIVE = 1,
        ACTIVE = 2,
        FREE = 3
    };

    struct EGID
    {
        public:
            uint entityID() const { return (uint)(_GID & 0xFFFFFFFF); }
            uint groupID() const { return (uint)(_GID >> 32); }
            EGID(const EGID& other) : _GID(other._GID) {}
            EGID(ulong identifier) : _GID(identifier) {}
            EGID(uint entityID, uint groupID) : _GID((ulong)groupID << 32 | ((ulong)(uint)entityID & 0xFFFFFFFF)) {}
            bool IsValid() const { return _GID > 0; }
            
            bool operator ==(const EGID& obj2) const { return _GID == obj2._GID; }
            bool operator !=(const EGID& obj2) const { return _GID != obj2._GID; }
            bool operator <(const EGID& obj2) const { return _GID < obj2._GID; }
            bool operator >(const EGID& obj2) const { return _GID > obj2._GID; }

        private:
            ulong _GID;
    };

    const EGID EGIDDefault = EGID(1);
    const EGID EGIDInvalid = EGID(0);

    struct IImplementer
    {
        virtual ~IImplementer() = 0 {}
    };

    struct IEntityView
    {
        EGID GID;
        virtual ~IEntityView() = 0 {}
    };
    
    const uint PK_ECS_BUCKET_SIZE = 32000;

    struct ImplementerBucket
    {
        void (*destructor)(void* value);
        void* data;

        ~ImplementerBucket() { destructor(data); }
    };

    struct ImplementerContainer
    {
        size_t count = 0;
        std::vector<Scope<ImplementerBucket>> buckets;
    };

    class EntityDatabase : public IService
    {
        public:
            int ReserveEntityId() { return ++m_idCounter; }

            template<typename T>
            T* ResereveImplementer()
            {
                auto type = std::type_index(typeid(T));
                auto& container = m_implementerBuckets[type];

                size_t elementsPerBucket = PK_ECS_BUCKET_SIZE / sizeof(T);
                size_t bucketIndex = container.count / elementsPerBucket;
                size_t subIndex = container.count - bucketIndex * elementsPerBucket;
                ++container.count;

                if (container.buckets.size() <= bucketIndex)
                {
                    auto newBucket = new ImplementerBucket();
                    newBucket->data = new T[elementsPerBucket];
                    newBucket->destructor = [](void* v) { delete[] reinterpret_cast<T*>(v); };
                    container.buckets.push_back(Scope<ImplementerBucket>(newBucket));
                }

                return reinterpret_cast<T*>(container.buckets.at(bucketIndex).get()->data) + subIndex;
            }

            template<typename T>
            T* ReserveEntityView(const EGID& egid)
            {
                PK_CORE_ASSERT(egid.IsValid(), "Trying to acquire resources for an invalid egid!");

                auto size = sizeof(T);
                auto type = std::type_index(typeid(T));
                auto& views = m_entityViews[type][egid.groupID()];
                auto offset = views.size();
                views.resize(offset + size);

                m_viewIndices[type][egid.entityID()] = offset;

                auto* element = reinterpret_cast<T*>(views.data() + offset);

                element->GID = egid;
                
                return element;
            }

            template<typename T>
            const BufferView<T> Query(const uint group)
            {
                PK_CORE_ASSERT(group, "Trying to acquire resources for an invalid egid!");
                auto size = sizeof(T);
                auto type = std::type_index(typeid(T));
                auto& views = m_entityViews[type][group];
                auto count = views.size() / size;
                return { reinterpret_cast<T*>(views.data()), count };
            }

            template<typename T>
            T* Query(const EGID& egid)
            {
                PK_CORE_ASSERT(egid.IsValid(), "Trying to acquire resources for an invalid egid!");
                auto size = sizeof(T);
                auto type = std::type_index(typeid(T));
                auto& views = m_entityViews.at(type).at(egid.groupID());
                auto offset = m_viewIndices.at(type).at(egid.entityID());
                return reinterpret_cast<T*>(views.data() + offset);
            }

        private:
            // @TODO refactor this to use better data structures
            std::map<std::type_index, std::map<uint, size_t>> m_viewIndices;
            std::map<std::type_index, std::map<uint, std::vector<char>>> m_entityViews;
            std::map<std::type_index, ImplementerContainer> m_implementerBuckets;
            int m_idCounter = 0;
    };
}