#include <unordered_map>
#include <vector>
#include <fstream>
#include <DxLib.h>
#include "../Libs/nlohmann/json.hpp"
#include "../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

// namespaceの省略
using Json = nlohmann::json;

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	// 静的インスタンスが生成されていなければ生成する
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

// リソース対応マップ 
static const std::unordered_map<Resource::TYPE, std::wstring> RESOURCE_TYPE_MAP =
{
	{Resource::TYPE::IMG, Application::PATH_IMAGE},
	{Resource::TYPE::FONT, Application::PATH_FONT},
	{Resource::TYPE::JSON, Application::PATH_JSON},
	{Resource::TYPE::EFFEKSEER, Application::PATH_EFFECT},
	{Resource::TYPE::MODEL, Application::PATH_MODEL},
	{Resource::TYPE::SOUND, Application::PATH_SOUND},
};

void ResourceManager::Init(void)
{
	// ローカル変数定義
	Resource::TYPE resoutceType = Resource::TYPE::NONE;
	std::wstring key = L"";
	std::wstring path = L"";

	// JSONファイル読み込み
	std::ifstream ifs((Application::PATH_JSON + L"Resources.json").c_str());

	// 読み込めない場合アサート
	assert(ifs.is_open() && L"ファイルが開けません");

	// データ格納
	Json resourceData = Json::parse(ifs, nullptr, true, true);

	// 各リソース登録
	for (auto& data : resourceData["resource"])
	{
		// 共通情報の格納
		resoutceType = static_cast<Resource::TYPE>(data["type"].get<int>());
		key = Utf8ToWide(data["key"].get<std::string>());
		path = Utf8ToWide(data["path"].get<std::string>());

		std::wstring basePath = L"";
		auto it = RESOURCE_TYPE_MAP.find(resoutceType);
		if (it != RESOURCE_TYPE_MAP.end())
		{
			basePath = it->second;
		}
		std::wstring fullPath = path.empty() ? L"" : basePath + path;

		Resource* newRes = new Resource(resoutceType, fullPath);
		resourcesMap_.emplace(key, newRes);
	}
}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	for (auto& res : resourcesMap_)
	{
		res.second->Release();
		delete res.second;
	}
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(std::wstring src)
{
	const Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(std::wstring src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	// 非同期読み込み中のモデルは複製できないため、読み込み完了を待つ
	if (CheckHandleASyncLoad(res.handleId_) == TRUE)
	{
		WaitHandleASyncLoad(res.handleId_);
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

	return duId;
}

Json& ResourceManager::GetJson(std::wstring src)
{
	Resource& res = _Load(src);
	return res.jsonData_;
}

bool ResourceManager::SaveJson(std::wstring src)
{
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		return false;
	}
	return rPair->second->Save();
}

bool ResourceManager::ReloadJson(std::wstring src)
{
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		return false;
	}
	rPair->second->Load();
	return true;
}

std::wstring ResourceManager::Utf8ToWide(const std::string& str)
{
	if (str.empty()) return L"";
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	if (size <= 1) return L"";
	std::wstring wstr(size - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
	return wstr;
}

ResourceManager::ResourceManager(void){}

Resource& ResourceManager::_Load(std::wstring src)
{
	// ロード済みチェック
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return *resourcesMap_.find(src)->second;
	}

	// リソース登録チェック
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		// 登録されていない
		return dummy_;
	}

	// ロード処理
	rPair->second->Load();

	// 念のためコピーコンストラクタ
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;
}
