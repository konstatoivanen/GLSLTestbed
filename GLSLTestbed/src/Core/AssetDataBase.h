#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/StringHashID.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/Ref.h"
#include "Utilities/Log.h"
#include "Core/ServiceRegister.h"
#include <filesystem>

typedef uint32_t AssetID;

class Asset
{
    friend class AssetDatabase;

    public:
        virtual ~Asset() = default;

        AssetID GetAssetID() const { return m_assetId; }

        const std::string& GetFileName() const { return StringHashID::IDToString(m_assetId); }

        bool IsFileAsset() const { return m_assetId != 0; }

        bool operator==(const Asset& other) const { return m_assetId == ((Asset&)other).m_assetId; }

    private:
        AssetID m_assetId = 0;
};

namespace AssetImporters
{
    template<typename T>
    void Import(const std::string& filepath, Ref<T>& asset);
};

class AssetDatabase : public IService
{
    private:
        template<typename T>
        Weak<T> Load(const std::string& filepath, AssetID assetId)
        {
            auto& collection = m_assets[std::type_index(typeid(T))];

            if (collection.count(assetId) > 0)
            {
                return std::static_pointer_cast<T>(collection.at(assetId));
            }

            auto asset = CreateRef<T>();
            collection[assetId] = asset;
            std::static_pointer_cast<Asset>(asset)->m_assetId = assetId;

            AssetImporters::Import<T>(filepath, asset);

            return asset;
        }

        template<typename T>
        Weak<T> Reload(const std::string& filepath, AssetID assetId)
        {
            auto& collection = m_assets[std::type_index(typeid(T))];
            Ref<T> asset = nullptr;

            if (collection.count(assetId) > 0)
            {
                asset = std::static_pointer_cast<T>(collection.at(assetId));
            }
            else
            {
                auto asset = CreateRef<T>();
                collection[assetId] = asset;
                std::static_pointer_cast<Asset>(asset)->m_assetId = assetId;
            }

            AssetImporters::Import<T>(filepath, asset);

            return asset;
        }

    public:
        template<typename T>
        Weak<T> Find(const char* name)
        {
            auto type = std::type_index(typeid(T));

            if (m_assets.count(type) > 0)
            {
                auto& collection = m_assets.at(type);

                for (auto& i : collection)
                {
                    auto filename = StringUtilities::ReadFileName(StringHashID::IDToString(i.first));

                    if (filename.find(name) != std::string::npos)
                    {
                        return std::static_pointer_cast<T>(i.second);
                    }
                }
            }

            PK_CORE_ERROR("Could not find asset with name %s", name);
        }
        
        template<typename T>
        Weak<T> Load(const std::string& filepath) { return Load<T>(filepath, StringHashID::StringToID(filepath)); }

        template<typename T>
        Weak<T> Load(AssetID assetId) { return Load<T>(StringHashID::IDToString(assetId), assetId); }

        template<typename T>
        Weak<T> Reload(const std::string& filepath) { return Reload<T>(filepath, StringHashID::StringToID(filepath)); }

        template<typename T>
        Weak<T> Reload(AssetID assetId) { return Reload<T>(StringHashID::IDToString(assetId), assetId); }

        template<typename T>
        void Reload(const Weak<T>& asset) 
        {
            auto assetId = asset.lock()->GetAssetID();
            return Reload<T>(StringHashID::IDToString(assetId), assetId); 
        }

        template<typename T>
        void LoadDirectory(const std::string& directory, std::initializer_list<const char*> extensions)
        {
            for (const auto& entry : std::filesystem::directory_iterator(directory))
            {
                auto& path = entry.path();

                if (!path.has_extension())
                {
                    continue;
                }

                auto pathExtension = path.extension();

                for (auto extension : extensions)
                {
                    if (pathExtension.compare(extension) == 0)
                    {
                        Load<T>(entry.path().string());
                        break;
                    }
                }
            }
        }

        template<typename T>
        void ReloadDirectory(const std::string& directory, std::initializer_list<const char*> extensions)
        {
            for (const auto& entry : std::filesystem::directory_iterator(directory))
            {
                auto& path = entry.path();

                if (!path.has_extension())
                {
                    continue;
                }

                auto pathExtension = path.extension();

                for (auto extension : extensions)
                {
                    if (pathExtension.compare(extension) == 0)
                    {
                        Reload<T>(entry.path().string());
                        break;
                    }
                }
            }
        }

        template<typename T>
        void UnloadDirectory(const std::string& directory, std::initializer_list<const char*> extensions)
        {
            for (const auto& entry : std::filesystem::directory_iterator(directory))
            {
                auto& path = entry.path();

                if (!path.has_extension())
                {
                    continue;
                }

                auto pathExtension = path.extension();

                for (auto extension : extensions)
                {
                    if (pathExtension.compare(extension) == 0)
                    {
                        Unload<T>(entry.path().string());
                        break;
                    }
                }
            }
        }

        template<typename T>
        void Unload(AssetID assetId)
        {
            auto& collection = m_assets[std::type_index(typeid(T))];
            collection.erase(assetId);
        }

        template<typename T>
        void Unload(const std::string& filepath)
        {
            Unload<T>(StringHashID::StringToID(filepath));
        }

        template<typename T>
        void Unload()
        {
            m_assets.erase(std::type_index(typeid(T)));
        }

        const void Unload() { m_assets.clear(); };

    private:
        std::unordered_map<std::type_index, std::unordered_map<AssetID, Ref<Asset>>> m_assets;
};

