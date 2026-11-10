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
	// 同名オブジェクトが既に登録されていれば上書きする
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
	// ステータスメッセージのタイマー更新
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
			if (obj.saveToJson)
			{
				Json& json = resMng.GetJson(obj.jsonSrc);
				obj.saveToJson(json);
				resMng.SaveJson(obj.jsonSrc);
			}
		}
		statusMessage_ = "Saved All Objects to JSON!";
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
		bool isSelected = (selectedIndex_ == i);
		if (ImGui::Selectable(objects_[i].name.c_str(), isSelected))
		{
			selectedIndex_ = i;
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// 右ペイン：選択中オブジェクトのパラメータ編集
	ImGui::BeginChild("ObjectInspector", ImVec2(0.0f, 0.0f), true);

	if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(objects_.size()))
	{
		auto& target = objects_[selectedIndex_];
		auto& resMng = ResourceManager::GetInstance();

		ImGui::Text("Target: %s", target.name.c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 230.0f);

		// Saveボタン
		if (ImGui::Button("Save to JSON", ImVec2(105.0f, 0.0f)))
		{
			if (target.saveToJson)
			{
				Json& json = resMng.GetJson(target.jsonSrc);
				target.saveToJson(json);
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
		}

		ImGui::SameLine();

		// Reloadボタン
		if (ImGui::Button("Reload JSON", ImVec2(105.0f, 0.0f)))
		{
			if (resMng.ReloadJson(target.jsonSrc) && target.loadFromJson)
			{
				const Json& json = resMng.GetJson(target.jsonSrc);
				target.loadFromJson(json);
				statusMessage_ = "Reloaded [" + target.name + "] from JSON!";
				statusTimer_ = STATUS_DISPLAY_TIME;
			}
		}

		ImGui::Separator();

		// オブジェクト固有のGUI描画(即時反映)
		if (target.drawGui)
		{
			target.drawGui();
		}
	}

	ImGui::EndChild();
	ImGui::End();
#endif
}
