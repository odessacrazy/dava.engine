#pragma once

#include <memory>
#include "NMaterialNames.h"
#include "NMaterialStateDynamicFlagsInsp.h"
#include "NMaterialStateDynamicPropertiesInsp.h"
#include "NMaterialStateDynamicTexturesInsp.h"
#include "Render/Shader/ShaderDescriptor.h"
#include "Render/Material/FXAsset.h"
#include "Scene3D/DataNode.h"

#include "Reflection/Reflection.h"
#include "Reflection/ReflectedMeta.h"

#include "MemoryManager/MemoryProfiler.h"
#include "Asset/AssetListener.h"

namespace DAVA
{
struct MaterialBufferBinding;

struct NMaterialProperty
{
    FastName name;
    rhi::ShaderProp::Type type;
    uint32 arraySize;
    uint32 updateSemantic;
    std::unique_ptr<float32[]> data;

    inline void SetPropertyValue(const float32* newValue);

    inline static uint32 GetCurrentUpdateSemantic()
    {
        return globalPropertyUpdateSemanticCounter;
    }

private:
    static uint32 globalPropertyUpdateSemanticCounter;
};

struct MaterialTextureInfo
{
    Asset<Texture> texture;
    FilePath path;
};

struct MaterialConfig
{
    MaterialConfig();
    ~MaterialConfig();
    MaterialConfig& operator=(const MaterialConfig& config);
    MaterialConfig(const MaterialConfig& config);

    void Clear();

    FastName name;
    FastName fxName;
    UnorderedMap<FastName, NMaterialProperty*> localProperties;
    UnorderedMap<FastName, MaterialTextureInfo*> localTextures;
    UnorderedMap<FastName, int32> localFlags; // integer flags are just more generic than boolean (eg. #if SHADING == HIGH), it has nothing in common with eFlagValue
};

class RenderVariantInstance
{
    friend class NMaterial;
    Asset<ShaderDescriptor> shader = nullptr;

    rhi::HDepthStencilState depthState, invZDepthStzte;
    rhi::HSamplerState samplerState;
    rhi::HTextureSet textureSet;
    rhi::CullMode cullMode, invCullMode;

    Vector<rhi::HConstBuffer> vertexConstBuffers;
    Vector<rhi::HConstBuffer> fragmentConstBuffers;

    Vector<MaterialBufferBinding*> materialBufferBindings;

    static const uint32 MAX_DYNAMIC_TEXTURE_PER_SHADER = 3;
    struct DynamicTextureBindInfo
    {
        uint32 index : 31;
        uint32 isVertex : 1;
        DynamicBindings::eTextureSemantic semantic;
        rhi::TextureType type;
    };
    uint32 dynamicTextureCount = 0;
    DynamicTextureBindInfo dynamicTextureBindInfo[MAX_DYNAMIC_TEXTURE_PER_SHADER];

    uint32 renderLayer = 0;
    bool wireFrame = false;
    bool alphablend = false;
    bool alphatest = false;

    RenderVariantInstance() = default;
    RenderVariantInstance(const RenderVariantInstance&) = delete;
    ~RenderVariantInstance();
};

class NMaterial : public DataNode, public AssetListener
{
    static Set<NMaterial*> allMaterialSet;

    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_NMATERIAL)

public:
    enum eBindFlags
    {
        FLAG_INV_Z = 1 << 0,
        FLAG_INV_CULL = 1 << 1,
        FLAG_INSTANCE = 1 << 2,
    };

    NMaterial(FXDescriptor::eType type = FXDescriptor::TYPE_LEGACY);
    ~NMaterial() override;

    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;

    NMaterial* Clone(FXDescriptor::eType newType = FXDescriptor::TYPE_COUNT);

    FXDescriptor::eType GetMaterialType() const;

    void SetFXName(const FastName& fxName);
    bool HasLocalFXName() const;
    const FastName& GetLocalFXName() const;
    const FastName& GetEffectiveFXName() const;

    const FastName& GetQualityGroup();
    void SetQualityGroup(const FastName& quality);

    inline void SetMaterialName(const FastName& name);
    inline const FastName& GetMaterialName() const;

    uint32 GetRequiredVertexFormat();

    void InvalidateBufferBindings();
    void InvalidateTextureBindings();
    void InvalidateRenderVariants();

    // properties
    void AddProperty(const FastName& propName, const float32* propData, rhi::ShaderProp::Type type, uint32 arraySize = 1);
    void RemoveProperty(const FastName& propName);
    void SetPropertyValue(const FastName& propName, const float32* propData);
    bool HasLocalProperty(const FastName& propName);
    bool HasEffectiveProperty(const FastName& propName);
    rhi::ShaderProp::Type GetLocalPropType(const FastName& propName);
    rhi::ShaderProp::Type GetEffectivePropType(const FastName& propName);
    uint32 GetLocalPropArraySize(const FastName& propName);
    const float32* GetLocalPropValue(const FastName& propName);
    const float32* GetEffectivePropValue(const FastName& propName);
    const UnorderedMap<FastName, NMaterialProperty*>& GetLocalProperties() const;

    // textures
    void AddTexture(const FastName& slotName, const Asset<Texture>& texture);
    void RemoveTexture(const FastName& slotName);
    void SetTexture(const FastName& slotName, const Asset<Texture>& texture);
    bool HasLocalTexture(const FastName& slotName);
    Asset<Texture> GetLocalTexture(const FastName& slotName);
    Asset<Texture> GetEffectiveTexture(const FastName& slotName);
    MaterialTextureInfo* GetLocalTextureInfo(const FastName& slotName);
    MaterialTextureInfo* GetEffectiveTextureInfo(const FastName& slotName);

    void CollectLocalTextures(Set<MaterialTextureInfo*>& collection) const;
    void CollectActiveLocalTextures(Set<MaterialTextureInfo*>& collection) const;
    bool ContainsTexture(const Asset<Texture>& texture) const;
    const UnorderedMap<FastName, MaterialTextureInfo*>& GetLocalTextures() const;

    // flags
    void AddFlag(const FastName& flagName, int32 value);
    void RemoveFlag(const FastName& flagName);
    void SetFlag(const FastName& flagName, int32 value);
    bool HasLocalFlag(const FastName& flagName);
    const UnorderedMap<FastName, int32>& GetLocalFlags() const;

    int32 GetLocalFlagValue(const FastName& flagName);
    int32 GetEffectiveFlagValue(const FastName& flagName);

    void SetParent(NMaterial* parent);
    NMaterial* GetParent() const;
    NMaterial* GetTopLevelParent();
    const Vector<NMaterial*>& GetChildren() const;

    inline uint32 GetRenderLayerID() const;
    inline uint32 GetSortingKey() const;

    //Configs managment
    uint32 GetConfigCount() const;
    const MaterialConfig& GetConfig(uint32 index) const;
    void InsertConfig(uint32 index, const MaterialConfig& config);
    void RemoveConfig(uint32 index);

    uint32 GetCurrentConfigIndex() const;
    void SetCurrentConfigIndex(uint32 index);

    const FastName& GetConfigName(uint32 index) const;
    void SetConfigName(uint32 index, const FastName& name);
    uint32 FindConfigByName(const FastName& name) const; //return size if config not found!
    const FastName& GetCurrentConfigName() const;
    void SetCurrentConfigName(const FastName& newName);

    void ReleaseConfigTextures(uint32 index);

    void BindParams(rhi::Packet& target, uint32 bindFlags = 0);

    // returns true if has variant for this pass, false otherwise
    // if material doesn't support pass active variant will be not changed
    // later add engine flags here
    bool PreBuildMaterial(const FastName& passName);

    // RHI_COMPLETE - it's temporary solution to avoid FX loading and shaders compilation after loading
    void PreCacheFX();
    void PreCacheFXWithFlags(const UnorderedMap<FastName, int32>& extraFlags, const FastName& extraFxName = FastName());
    void PreCacheFXVariations(const Vector<FastName>& fxNames, const Vector<FastName>& flags);

    enum eUserFlag
    {
        USER_FLAG_ALPHABLEND = 1 << 0,
        USER_FLAG_ALPHATEST = 1 << 1,
    };

    // AssetListener
    void OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset) override;

    Asset<FXAsset> GetFXAsset() const
    {
        return fxAsset;
    }

private:
    void LoadOldNMaterial(KeyedArchive* archive, SerializationContext* serializationContext);
    void SaveConfigToArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext, bool forceNameSaving);
    void LoadConfigFromArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext);

    void RebuildBindings();
    void RebuildTextureBindings();
    void RebuildRenderVariants();

    bool NeedLocalOverride(UniquePropertyLayout propertyLayout);
    void ClearLocalBuffers();
    void InjectChildBuffer(UniquePropertyLayout propLayoutId, MaterialBufferBinding* buffer);

    // the following functions will collect data recursively
    MaterialBufferBinding* GetConstBufferBinding(UniquePropertyLayout propertyLayout);
    NMaterialProperty* GetMaterialProperty(const FastName& propName);
    void CollectMaterialFlags(UnorderedMap<FastName, int32>& target);
    void CollectConfigTextures(const MaterialConfig& config, Set<MaterialTextureInfo*>& collection) const;

    void AddChildMaterial(NMaterial* material);
    void RemoveChildMaterial(NMaterial* material);

    const MaterialConfig& GetCurrentConfig() const;
    MaterialConfig& GetMutableCurrentConfig();
    MaterialConfig& GetMutableConfig(uint32 index);

    const FastName& GetActiveVariantName() const;

private:
    // config time
    FastName materialName;
    FastName qualityGroup;
    FXDescriptor::eType materialType = FXDescriptor::TYPE_LEGACY;

    Vector<MaterialConfig> materialConfigs;

    // runtime
    NMaterial* parent = nullptr;
    Vector<NMaterial*> children;

    uint32 currentConfig = 0;

    Asset<FXAsset> fxAsset;

    FastName activeVariantName;
    RenderVariantInstance* activeVariantInstance = nullptr;

    UnorderedMap<UniquePropertyLayout, MaterialBufferBinding*> localConstBuffers;

    // this is for render passes - not used right now - only active variant instance
    UnorderedMap<FastName, RenderVariantInstance*> renderVariants;

    uint32 sortingKey = 0;
    bool needRebuildBindings = true;
    bool needRebuildTextures = true;
    bool needRebuildVariants = true;

    friend class NMaterialStateDynamicFlagsInsp;
    friend class NMaterialStateDynamicPropertiesInsp;
    friend class NMaterialStateDynamicTexturesInsp;

    friend class NMaterialBaseStructureWrapper;

    DAVA_VIRTUAL_REFLECTION(NMaterial, DataNode);
};

namespace Metas
{
struct MaterialType
{
    MaterialType(FXDescriptor::eType type_)
        : type(type_)
    {
    }

    FXDescriptor::eType type;
};
}

namespace M
{
using MaterialType = Meta<Metas::MaterialType>;
}

void NMaterialProperty::SetPropertyValue(const float32* newValue)
{
    //4 is because register size is float4
    Memcpy(data.get(), newValue, sizeof(float32) * ShaderDescriptor::CalculateDataSize(type, arraySize));
    updateSemantic = ++globalPropertyUpdateSemanticCounter;
}

void NMaterial::SetMaterialName(const FastName& name)
{
    materialName = name;
}

const FastName& NMaterial::GetMaterialName() const
{
    return materialName;
}

uint32 NMaterial::GetRenderLayerID() const
{
    if (activeVariantInstance)
        return activeVariantInstance->renderLayer;
    else
        return static_cast<uint32>(-1);
}

uint32 NMaterial::GetSortingKey() const
{
    return sortingKey;
}

inline uint32 NMaterial::GetCurrentConfigIndex() const
{
    return currentConfig;
}

inline const MaterialConfig& NMaterial::GetCurrentConfig() const
{
    return GetConfig(GetCurrentConfigIndex());
}

inline MaterialConfig& NMaterial::GetMutableCurrentConfig()
{
    return GetMutableConfig(GetCurrentConfigIndex());
}

inline const MaterialConfig& NMaterial::GetConfig(uint32 index) const
{
    DVASSERT(index < materialConfigs.size());
    return materialConfigs[index];
}

inline MaterialConfig& NMaterial::GetMutableConfig(uint32 index)
{
    DVASSERT(index < materialConfigs.size());
    return materialConfigs[index];
}
};
