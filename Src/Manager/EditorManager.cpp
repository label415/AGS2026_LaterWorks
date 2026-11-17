#include <algorithm>
#include "../Libs/ImGui/imgui.h"
#include "SceneManager.h"
#include "EditorManager.h"

EditorManager::EditorManager(void)
	: selectedIndex_(0)
	, isPauseGame_(false)
	, isDrawCollider_(true)
	, statusMessage_("")
	, statusTimer_(0.0f)
{
}

void EditorManager::RegisterObject(const EditObject& obj)
{
#ifdef _DEBUG
	for (auto& existing : objects_)
	{
		if (existing.name == obj.name)
		{
			existing = obj;
			return;
		}
	}
	objects_.push_back(obj);
#endif
}

void EditorManager::UnregisterObject(const std::string& name)
{
#ifdef _DEBUG
	objects_.erase(
		std::remove_if(objects_.begin(), objects_.end(),
			[&name](const EditObject& o) { return o.name == name; }),
		objects_.end());

	if (selectedIndex_ >= static_cast<int>(objects_.size()))
	{
		selectedIndex_ = (std::max)(0, static_cast<int>(objects_.size()) - 1);
	}
#endif
}

void EditorManager::ClearObjects(void)
{
	objects_.clear();
	selectedIndex_ = 0;
}

void EditorManager::Release(void)
{
	ClearObjects();
}

void EditorManager::UpdateGui(void)
{
#ifdef _DEBUG
	if (statusTimer_ > 0.0f)
	{
		statusTimer_ -= SceneManager::GetInstance().GetDeltaTime();
	}

	ImGui::SetNextWindowSize(ImVec2(680.0f, 620.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(20.0f, 80.0f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Game Data Editor [Key:1 Open / Key:2 Close]"))
	{
		ImGui::End();
		return;
	}

	// 上部ツールバー
	ImGui::Checkbox("Pause Game", &isPauseGame_);
	ImGui::SameLine();
	ImGui::Checkbox("Draw Colliders", &isDrawCollider_);
	ImGui::SameLine();

	if (ImGui::Button("Save All"))
	{
		auto& resMng = ResourceManager::GetInstance();
		for (auto& obj : objects_)
		{
			resMng.SaveJson(obj.jsonSrc);
		}
		statusMessage_ = "Saved All JSON Files!";
		statusTimer_ = STATUS_DISPLAY_TIME;
	}

	if (statusTimer_ > 0.0f && !statusMessage_.empty())
	{
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "%s", statusMessage_.c_str());
	}

	ImGui::Separator();

	if (objects_.empty())
	{
		ImGui::Text("No editable objects registered.");
		ImGui::End();
		return;
	}

	// 左ペイン：オブジェクト一覧
	ImGui::BeginChild("ObjectList", ImVec2(LEFT_PANE_WIDTH, 0.0f), true);
	ImGui::Text("Objects");
	ImGui::Separator();

	for (int i = 0; i < static_cast<int>(objects_.size()); ++i)
	{
		if (ImGui::Selectable(objects_[i].name.c_str(), selectedIndex_ == i))
		{
			selectedIndex_ = i;
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// 右ペイン：選択中オブジェクトのJSON直接編集
	ImGui::BeginChild("ObjectInspector", ImVec2(0.0f, 0.0f), true);

	if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(objects_.size()))
	{
		auto& target = objects_[selectedIndex_];
		auto& resMng = ResourceManager::GetInstance();
		Json& json = resMng.GetJson(target.jsonSrc);

		ImGui::Text("Target: %s", target.name.c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 230.0f);

		// JSONファイルへ直接保存
		if (ImGui::Button("Save to JSON", ImVec2(105.0f, 0.0f)))
		{
			if (resMng.SaveJson(target.jsonSrc))
			{
				statusMessage_ = "Saved [" + target.name + "] to JSON!";
			}
			else
			{
				statusMessage_ = "Failed to save [" + target.name + "]";
			}
			statusTimer_ = STATUS_DISPLAY_TIME;
		}

		ImGui::SameLine();

		// JSONファイルから直接再読込して即時反映
		if (ImGui::Button("Reload JSON", ImVec2(105.0f, 0.0f)))
		{
			if (resMng.ReloadJson(target.jsonSrc))
			{
				if (target.onApplyJson) target.onApplyJson();
				statusMessage_ = "Reloaded [" + target.name + "] from JSON!";
				statusTimer_ = STATUS_DISPLAY_TIME;
			}
		}

		ImGui::Separator();

		// ランタイム専用の補助GUI(現在座標や現在HPなど)があれば表示
		if (target.drawRuntimeGui)
		{
			target.drawRuntimeGui();
		}

		// JSONデータを直接ImGuiで描画・編集し、変更があれば即座にゲームへ反映
		if (DrawJsonEditor(json))
		{
			if (target.onApplyJson)
			{
				target.onApplyJson();
			}
		}
	}

	ImGui::EndChild();
	ImGui::End();
#endif
}

bool EditorManager::DrawJsonEditor(Json& json)
{
	bool changed = false;
	if (!json.is_object()) return false;

	for (auto it = json.begin(); it != json.end(); ++it)
	{
		if (DrawJsonNode(it.key(), it.value(), true))
		{
			changed = true;
		}
	}
	return changed;
}

bool EditorManager::DrawJsonNode(const std::string& key, Json& node, bool isTopLevel)
{
	bool changed = false;
	ImGui::PushID(key.c_str());

	if (node.is_object())
	{
		// { "x", "y", "z" } の3要素オブジェクトはDragFloat3として1行で編集
		if (node.size() == 3 && node.contains("x") && node.contains("y") && node.contains("z")
			&& node["x"].is_number() && node["y"].is_number() && node["z"].is_number())
		{
			float v[3] = {
				node["x"].get<float>(),
				node["y"].get<float>(),
				node["z"].get<float>()
			};
			if (ImGui::DragFloat3(key.c_str(), v, 0.5f))
			{
				node["x"] = v[0];
				node["y"] = v[1];
				node["z"] = v[2];
				changed = true;
			}
		}
		else if (isTopLevel)
		{
			if (ImGui::CollapsingHeader(key.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (auto it = node.begin(); it != node.end(); ++it)
				{
					if (DrawJsonNode(it.key(), it.value(), false)) changed = true;
				}
			}
		}
		else
		{
			if (ImGui::TreeNodeEx(key.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (auto it = node.begin(); it != node.end(); ++it)
				{
					if (DrawJsonNode(it.key(), it.value(), false)) changed = true;
				}
				ImGui::TreePop();
			}
		}
	}
	else if (node.is_array())
	{
		if (ImGui::CollapsingHeader(key.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (size_t i = 0; i < node.size(); ++i)
			{
				std::string itemLabel = "[" + std::to_string(i) + "]";
				if (node[i].is_object() && node[i].contains("name") && node[i]["name"].is_string())
				{
					itemLabel = node[i]["name"].get<std::string>() + " [" + std::to_string(i) + "]";
				}
				if (DrawJsonNode(itemLabel, node[i], false)) changed = true;
			}
		}
	}
	else if (node.is_number_integer())
	{
		int val = node.get<int>();
		if (ImGui::InputInt(key.c_str(), &val))
		{
			node = val;
			changed = true;
		}
	}
	else if (node.is_number())
	{
		float val = node.get<float>();
		if (ImGui::DragFloat(key.c_str(), &val, 0.2f))
		{
			node = val;
			changed = true;
		}
	}
	else if (node.is_boolean())
	{
		bool val = node.get<bool>();
		if (ImGui::Checkbox(key.c_str(), &val))
		{
			node = val;
			changed = true;
		}
	}

	ImGui::PopID();
	return changed;
}
