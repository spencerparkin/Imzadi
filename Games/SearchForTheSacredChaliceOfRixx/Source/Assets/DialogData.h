#pragma once

#include "AssetCache.h"
#include <unordered_map>
#include <string>

class DialogSequence;
class DialogTurn;

class DialogData : public Imzadi::Asset
{
public:
	DialogData();
	virtual ~DialogData();

	virtual bool Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache) override;
	virtual bool Unload() override;

	typedef std::unordered_map<std::string, Imzadi::Reference<DialogSequence>> SequenceMap;
	SequenceMap sequenceMap;
};

class DialogElement : public Imzadi::ReferenceCounted
{
public:
	DialogElement();
	virtual ~DialogElement();

	virtual bool Load(const rapidjson::Value& elementValue);
	virtual bool Setup() = 0;
	virtual bool Shutdown() = 0;
	virtual bool Tick(std::string& nextSequence, int& nextSequencePosition) = 0;

	std::string speaker;
};

class DialogBasicElement : public DialogElement
{
public:
	DialogBasicElement();
	virtual ~DialogBasicElement();

	virtual bool Load(const rapidjson::Value& elementValue) override;
	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(std::string& nextSequence, int& nextSequencePosition) override;

	std::string text;
	std::string sceneObjName;
};

class DialogChoiceElement : public DialogElement
{
public:
	DialogChoiceElement();
	virtual ~DialogChoiceElement();

	virtual bool Load(const rapidjson::Value& elementValue) override;
	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(std::string& nextSequence, int& nextSequencePosition) override;

	struct Choice
	{
		std::string text;
		std::string sequenceName;
	};

	std::vector<Choice> choiceArray;
};

class DialogAcquireElement : public DialogBasicElement
{
public:
	DialogAcquireElement();
	virtual ~DialogAcquireElement();

	virtual bool Load(const rapidjson::Value& elementValue) override;
	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(std::string& nextSequence, int& nextSequencePosition) override;

	std::string acquiredItem;
};

class DialogSequence : public Imzadi::ReferenceCounted
{
public:
	DialogSequence();
	virtual ~DialogSequence();

	std::vector<Imzadi::Reference<DialogElement>> dialogElementArray;
};