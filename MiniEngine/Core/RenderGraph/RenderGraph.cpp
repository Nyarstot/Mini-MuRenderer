#include "pch.h"
#include "RenderGraph/RenderGraph.h"
#include "Util/StringUtils.h"
#include "GraphicsCore.h"

#include <fstream>


namespace RenderGraph
{
    RenderGraph::RenderGraph(const std::wstring& name)
        : BaseGraph<RenderPass, RenderGraphEdgeResourceData>()
    {
        ASSERT_SUCCEEDED(Graphics::g_multiAdapterManager.GetDevice(0)->CreateFence(
            0, D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER, IID_PPV_ARGS(&m_sharedFence)
        ));

        this->SetTitle(name);
    }

    std::size_t RenderGraph::AddRenderPass(std::unique_ptr<RenderPass> renderPass)
    {
        return this->AddNode(std::move(renderPass));
    }

    ResourceEntry RenderGraph::GetRegisteredResourceEntry(const std::wstring& name) const
    {
        return m_resourceRegistry.GetRegisteredResourceEntry(name);
    }

    bool RenderGraph::IsResourceRegistered(const std::wstring& name) const
    {
        return m_resourceRegistry.IsResourceRegistered(name);
    }

    std::shared_ptr<RenderGraphResource> RenderGraph::CreateResource(const std::wstring& name, const RenderGraphResourceDesc& desc)
    {
        auto resource = std::make_shared<RenderGraphResource>(name, desc);
        //m_resources[name] = resource;
        m_resources.insert({ name, resource });
        return resource;
    }

    std::shared_ptr<RenderGraphResource> RenderGraph::GetResource(const std::wstring& name) const
    {
        auto it = m_resources.find(name);
        return it != m_resources.end() ? it->second : nullptr;
    }

    void RenderGraph::Compile()
    {

        /////////////////////
        // Topological sort
        /////////////////////

        m_executionOrder.clear();
        std::vector<std::size_t> inDegree(GetNodeCount(), 0);
        std::vector<std::size_t> readyNodes;

        // Calculate in-degree for each node
        for (std::size_t i = 0; i < GetNodeCount(); ++i) {
            inDegree[i] = GetNode(i).inputEdges.size();
        }

        // Find initial ready nodes (in-degree = 0)
        for (std::size_t i = 0; i < GetNodeCount(); ++i) {
            if (inDegree[i] == 0) {
                readyNodes.push_back(i);
            }
        }

        /////////////////////
        // Process
        /////////////////////

        while (!readyNodes.empty()) {
            std::size_t nodeId = readyNodes.back();
            readyNodes.pop_back();
            m_executionOrder.push_back(nodeId);

            // Update in-degree of neighbors
            const auto& node = GetNode(nodeId);
            for (std::size_t edgeId : node.outputEdges) {
                const auto& edge = GetEdge(edgeId);
                if (--inDegree[edge.toNode] == 0) {
                    readyNodes.push_back(edge.toNode);
                }
            }
        }

        // Check for cycles
        if (m_executionOrder.size() != GetNodeCount()) {
            throw std::runtime_error("Render graph contains cycles");
        }
    }

    void RenderGraph::Execute(CommandContext& ctx)
    {
        m_fenceValue = 1;

        for (std::size_t nodeId : m_executionOrder) {
            auto& node = GetNode(nodeId);
            if (node.data->IsMultiAdapterAllowed()) {
                node.data->SetSharedFence(m_sharedFence);
                node.data->Execute(ctx);
            }
            else
            {
                node.data->Execute(ctx);
            }
        }
    }

    void RenderGraph::Clear()
    {
        BaseGraph<RenderPass, RenderGraphEdgeResourceData>::Clear();
        m_resources.clear();
    }

    nlohmann::json RenderGraph::Serialize() const
    {
        nlohmann::json json;

        // Serialize resources

        nlohmann::json resourcesJson;
        for (const auto& resource : m_resources) {
            const auto& desc = resource.second->GetDescription();
            const auto& name = resource.first;

            nlohmann::json resourceJson;
            resourceJson["type"]            = static_cast<int>(desc.type);
            resourceJson["format"]          = static_cast<int>(desc.format);
            resourceJson["height"]          = desc.height;
            resourceJson["width"]           = desc.width;
            resourceJson["depth"]           = desc.depth;
            resourceJson["mip_levels"]      = desc.mipLevels;
            resourceJson["array_size"]      = desc.arraySize;
            resourceJson["sample_count"]    = desc.sampleCount;
            resourceJson["flags"]           = static_cast<int>(desc.flags);
            resourceJson["initial_state"]   = static_cast<int>(desc.initialState);

            resourcesJson[StringUtils::WideToNarrow(name)] = resourceJson;
        }
        json["resources"] = resourcesJson;

        // Serialize nodes
        nlohmann::json nodesJson;
        for (std::size_t i = 0; i < this->GetNodeCount(); ++i) {
            const auto& node = GetNode(i);
            nlohmann::json nodeJson;

            nodeJson["name"] = StringUtils::WideToNarrow(node.data->GetName());
            nodeJson["type"] = typeid(*node.data).name();

            // Serialize output edges
            nlohmann::json edgesJson;
            for (std::size_t edgeId : node.outputEdges) {
                const auto& edge = GetEdge(edgeId);
                nlohmann::json edgeJson;

                edgeJson["to_node"] = edge.toNode;
                edgeJson["data"]    = edge.data->Serialize();
                edgesJson.push_back(edgeJson);
            }
            nodeJson["output_edges"] = edgesJson;
            nodesJson.push_back(nodeJson);
        }
        json["nodes"] = nodesJson;

        // Serialize execution order
        if (!m_executionOrder.empty()) {
            json["execution_order"] = m_executionOrder;
        }

        return json;
    }

    void RenderGraph::ExportToJSON() const
    {
        std::ofstream out("render_graph.json");
        out << this->Serialize().dump();
        out.close();
    }

    D3D12_RESOURCE_STATES RenderGraph::GetCurrentState(const std::shared_ptr<RenderGraphResource>& resource) const
    {
        return resource->GetDescription().initialState;
    }

    RenderGraphStoraged::RenderGraphStoraged()
        : BaseGraphStoraged<RenderPass, RenderGraphEdgeResourceData>()
    {
    }

    std::shared_ptr<RenderGraphResource> RenderGraphStoraged::CreateResource(const std::wstring& name, const RenderGraphResourceDesc& desc)
    {
        auto resource = std::make_shared<RenderGraphResource>(name, desc);
        //m_resources[name] = resource;
        m_resources.insert({ name, resource });
        return resource;
    }

    std::shared_ptr<RenderGraphResource> RenderGraphStoraged::GetResource(const std::wstring& name) const
    {
        auto it = m_resources.find(name);
        return it != m_resources.end() ? it->second : nullptr;
    }

    void RenderGraphStoraged::Compile()
    {

        /////////////////////
        // Topological sort
        /////////////////////

        m_executionOrder.clear();
        std::vector<std::size_t> inDegree(GetNodeCount(), 0);
        std::vector<std::size_t> readyNodes;

        // Calculate in-degree for each node
        for (std::size_t i = 0; i < GetNodeCount(); ++i) {
            inDegree[i] = GetNode(i).inputEdges.size();
        }

        // Find initial ready nodes (in-degree = 0)
        for (std::size_t i = 0; i < GetNodeCount(); ++i) {
            if (inDegree[i] == 0) {
                readyNodes.push_back(i);
            }
        }

        /////////////////////
        // Process
        /////////////////////

        while (!readyNodes.empty()) {
            std::size_t nodeId = readyNodes.back();
            readyNodes.pop_back();
            m_executionOrder.push_back(nodeId);

            // Update in-degree of neighbors
            const auto& node = GetNode(nodeId);
            for (std::size_t edgeId : node.outputEdges) {
                const auto& edge = GetEdge(edgeId);
                if (--inDegree[edge.toNode] == 0) {
                    readyNodes.push_back(edge.toNode);
                }
            }
        }

        // Check for cycles
        if (m_executionOrder.size() != GetNodeCount()) {
            throw std::runtime_error("Render graph contains cycles");
        }
    }

    void RenderGraphStoraged::Execute(CommandContext& ctx)
    {
        for (std::size_t nodeId : m_executionOrder) {
            auto& node = GetNode(nodeId);
            m_nodeStorage.at(node.dataIndex)->Execute(ctx);
        }
    }

    void RenderGraphStoraged::Clear()
    {
        BaseGraphStoraged<RenderPass, RenderGraphEdgeResourceData>::Clear();
        // m_resources.clear();
    }

    nlohmann::json RenderGraphStoraged::Serialize() const
    {
        nlohmann::json json;

        // Serialize resources

        nlohmann::json resourcesJson;
        for (const auto& resource : m_resources) {
            const auto& desc = resource.second->GetDescription();
            const auto& name = resource.first;

            nlohmann::json resourceJson;
            resourceJson["type"]            = static_cast<int>(desc.type);
            resourceJson["format"]          = static_cast<int>(desc.format);
            resourceJson["height"]          = desc.height;
            resourceJson["width"]           = desc.width;
            resourceJson["depth"]           = desc.depth;
            resourceJson["mip_levels"]      = desc.mipLevels;
            resourceJson["array_size"]      = desc.arraySize;
            resourceJson["sample_count"]    = desc.sampleCount;
            resourceJson["flags"]           = static_cast<int>(desc.flags);
            resourceJson["initial_state"]   = static_cast<int>(desc.initialState);

            resourcesJson[StringUtils::WideToNarrow(name)] = resourceJson;
        }
        json["resources"] = resourcesJson;

        // Serialize nodes
        nlohmann::json nodesJson;
        for (std::size_t i = 0; i < this->GetNodeCount(); ++i) {
            const auto& node = GetNode(i);
            nlohmann::json nodeJson;

            nodeJson["name"] = StringUtils::WideToNarrow(node.dataIndex);
            nodeJson["type"] = typeid(*(m_nodeStorage.at(node.dataIndex))).name();

            // Serialize output edges
            nlohmann::json edgesJson;
            for (std::size_t edgeId : node.outputEdges) {
                const auto& edge = GetEdge(edgeId);
                nlohmann::json edgeJson;

                edgeJson["name"] = StringUtils::WideToNarrow(edge.dataIndex);
                edgeJson["to_node"] = edge.toNode;
                edgeJson["data"]    = m_edgeStorage.at(edge.dataIndex)->Serialize();
                edgesJson.push_back(edgeJson);
            }
            nodeJson["output_edges"] = edgesJson;
            nodesJson.push_back(nodeJson);
        }
        json["nodes"] = nodesJson;

        // Serialize execution order
        if (!m_executionOrder.empty()) {
            json["execution_order"] = m_executionOrder;
        }

        return json;
    }

    void RenderGraphStoraged::ExportToJSON() const
    {
        std::ofstream out("./render_graph.json");
        out << this->Serialize().dump();
        out.close();
    }

    RenderGraphStoraged* RenderGraphStoraged::Deserialize(const nlohmann::json& from)
    {
        RenderGraphStoraged* result = new RenderGraphStoraged();

        for (const auto& entry : from["resources"].items()) {
            RenderGraphResourceDesc desc = {
                static_cast<RenderGraphResourceType>(entry.value()["type"]),
                static_cast<DXGI_FORMAT>(entry.value()["format"]),
                static_cast<std::uint32_t>(entry.value()["height"]),
                static_cast<std::uint32_t>(entry.value()["width"]),
                static_cast<std::uint16_t>(entry.value()["depth"]),
                static_cast<std::uint16_t>(entry.value()["mip_levels"]),
                static_cast<std::uint16_t>(entry.value()["array_size"]),
                static_cast<std::uint32_t>(entry.value()["sample_count"]),
                static_cast<D3D12_RESOURCE_FLAGS>(entry.value()["flags"]),
                static_cast<D3D12_RESOURCE_STATES>(entry.value()["initial_state"]),
            };

            result->CreateResource(StringUtils::NarrowToWide(entry.key()), desc);
        }

        // for (const auto& [key, value] : from["nodes"].items()) {
            //
        // }

        if (from.contains("execution_order")) {
            result->m_executionOrder.clear();
            result->m_executionOrder.reserve(from["execution_order"].size());
            for (const auto& entry : from["execution_order"].items()) {
                result->m_executionOrder.push_back(entry.value());
            }
        }

        return result;
    }

    RenderGraphStoraged* RenderGraphStoraged::ImportFromJSON()
    {
        std::ifstream in("render_graph.json");
        nlohmann::json serialized = nlohmann::json::parse(in);
        return RenderGraphStoraged::Deserialize(serialized);
    }

    D3D12_RESOURCE_STATES RenderGraphStoraged::GetCurrentState(const std::shared_ptr<RenderGraphResource>& resource) const
    {
        return resource->GetDescription().initialState;
    }
}