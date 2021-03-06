//
// Copyright (c) 2017-2019 Rokas Kupstys.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#   define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <vector>
#include <imgui.h>
#include <SDL_mouse.h>
#include "ImNodes.h"

namespace ImGui
{

struct MyNode : ImNodes::NodeInfo
{
    explicit MyNode(const char* title, const std::vector<ImNodes::SlotInfo>& inputs, const std::vector<ImNodes::SlotInfo>& outputs)
    {
        this->title = title;
        this->inputs = inputs;
        this->outputs = outputs;
    }

    MyNode(const MyNode& other)
        : ImNodes::NodeInfo(other)
    {
        // Moves internal node state
        // ImNodes::CopyConstructed(&canvas, &other, this);

        inputs = other.inputs;
        outputs = other.outputs;
        connections = other.connections;

        // Connections point to old node. Loop and swap pointers to new node.
        for (auto& connection : connections)
        {
            if (connection.input_node == &other)
            {
                connection.input_node = this;
                for (auto& other_conn : ((MyNode*)connection.output_node)->connections)
                {
                    if (other_conn.output_node == &other)
                    {
                        other_conn.output_node = this;
                        break;
                    }
                }
            }
            else if (connection.output_node == &other)
            {
                connection.output_node = this;
                for (auto& other_conn : ((MyNode*)connection.input_node)->connections)
                {
                    if (other_conn.input_node == &other)
                    {
                        other_conn.input_node = this;
                        break;
                    }
                }
            }
        }
    }

    std::vector<ImNodes::SlotInfo> inputs;
    std::vector<ImNodes::SlotInfo> outputs;
    std::vector<ImNodes::Connection> connections;
};

std::vector<MyNode> available_nodes{
    MyNode("Node 0",
    {
        // Inputs
        {
            "Input 0",
        },
        {
            "Input longer",
        }
    },
    {
        // Outputs
        {
            "Output 0",
        },
        {
            "Output longer",
        }
    }),
    MyNode("Node 1",
    {
        // Inputs
        {
            "Input 0",
        }
    },
    {
        // Outputs
        {
            "Output longer",
        }
    })
};
std::vector<MyNode> nodes;
ImNodes::CanvasState canvas{};

void DeleteConnection(ImNodes::Connection* connection, std::vector<ImNodes::Connection>& connections)
{
    for (auto it = connections.begin(); it != connections.end(); ++it)
    {
        if (*connection == *it)
        {
            connections.erase(it);
            break;
        }
    }
}

void ShowDemoWindow(bool*)
{
    if (ImGui::Begin("ImNodes"))
    {
        // We probably need to keep some state, like positions of nodes/slots for rendering connections.
        ImNodes::BeginCanvas(&canvas);
        for (auto it = nodes.begin(); it != nodes.end();)
        {
            MyNode& node = *it;
            if (ImNodes::BeginNode(&node,
                &node.inputs[0], node.inputs.size(),
                &node.outputs[0], node.outputs.size(),
                &node.connections[0], node.connections.size()))
            {
                // Custom widgets can be rendered in the middle of node
                ImGui::TextUnformatted("node content");

                // Store new connections
                ImNodes::Connection new_connection;
                if (ImNodes::GetNewConnection(&new_connection))
                {
                    ((MyNode*) new_connection.input_node)->connections.push_back(new_connection);
                    ((MyNode*) new_connection.output_node)->connections.push_back(new_connection);
                }

                // Remove deleted connections
                if (ImNodes::Connection* connection = ImNodes::GetDeleteConnection())
                {
                    DeleteConnection(connection, ((MyNode*) connection->input_node)->connections);
                    DeleteConnection(connection, ((MyNode*) connection->output_node)->connections);
                }

                ImNodes::EndNode();
            }

            if (node.selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete))
            {
                for (auto& connection : node.connections)
                {
                    DeleteConnection(&connection, ((MyNode*) connection.input_node)->connections);
                    DeleteConnection(&connection, ((MyNode*) connection.output_node)->connections);
                }
                it = nodes.erase(it);
            }
            else
                ++it;
        }

        const ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsMouseReleased(1) && ImGui::IsMouseHoveringWindow() && !ImGui::IsMouseDragging(1))
            ImGui::OpenPopup("NodesContextMenu");

        if (ImGui::BeginPopup("NodesContextMenu"))
        {
            for (const auto& node_template : available_nodes)
            {
                if (ImGui::MenuItem(node_template.title))
                    nodes.emplace_back(node_template);
            }
            if (ImGui::IsAnyMouseDown() && !ImGui::IsMouseHoveringWindow())
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImNodes::EndCanvas();
    }
    ImGui::End();

}

}
