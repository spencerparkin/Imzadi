#include "DialogData.h"
#include "Log.h"

//------------------------------------ DialogData ------------------------------------

DialogData::DialogData()
{
}

/*virtual*/ DialogData::~DialogData()
{
}

/*virtual*/ bool DialogData::Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache)
{
	if (!jsonDoc.HasMember("sequence_table") || !jsonDoc["sequence_table"].IsObject())
	{
		IMZADI_LOG_ERROR("Expected \"sequence_table\" member to exist and be an object.");
		return false;
	}

	const rapidjson::Value& sequenceTableValue = jsonDoc["sequence_table"];
	for (auto iter = sequenceTableValue.MemberBegin(); iter != sequenceTableValue.MemberEnd(); ++iter)
	{
		std::string sequenceName = iter->name.GetString();

		const rapidjson::Value& sequenceValue = iter->value;
		if (!sequenceValue.IsObject())
		{
			IMZADI_LOG_ERROR("Expected all members of the \"sequence_table\" to be objects.");
			return false;
		}

		if (!sequenceValue.HasMember("sequence") || !sequenceValue["sequence"].IsArray())
		{
			IMZADI_LOG_ERROR("Expected \"sequence\" member to exist and be an array.");
			return false;
		}

		const rapidjson::Value& sequenceArrayValue = sequenceValue["sequence"];
		Imzadi::Reference<DialogSequence> dialogSequence(new DialogSequence());
		for (int i = 0; i < sequenceArrayValue.Size(); i++)
		{
			const rapidjson::Value& elementValue = sequenceArrayValue[i];
			if (!elementValue.IsObject())
			{
				IMZADI_LOG_ERROR("Expected \"sequence\" array member to be an object.");
				return false;
			}

			if (!elementValue.HasMember("type") || !elementValue["type"].IsString())
			{
				IMZADI_LOG_ERROR("Expected \"type\" member to exist or be a string in the \"sequence\" array.");
				return false;
			}

			Imzadi::Reference<DialogElement> element;

			std::string elementType = elementValue["type"].GetString();
			if (elementType == "basic")
				element.Set(new DialogBasicElement());
			else if (elementType == "choice")
				element.Set(new DialogChoiceElement());
			else if (elementType == "acquire")
				element.Set(new DialogAcquireElement());
			else
			{
				IMZADI_LOG_ERROR("Did not recognoze dialog element type: %s", elementType.c_str());
				return false;
			}

			if (!element->Load(elementValue))
			{
				IMZADI_LOG_ERROR("Failed to load dialog element.");
				return false;
			}

			dialogSequence->dialogElementArray.push_back(element);
		}

		this->sequenceMap.insert(std::pair<std::string, Imzadi::Reference<DialogSequence>>(sequenceName, dialogSequence));
	}

	return true;
}

/*virtual*/ bool DialogData::Unload()
{
	this->sequenceMap.clear();
	return true;
}

//------------------------------------ DialogData ------------------------------------

DialogElement::DialogElement()
{
}

/*virtual*/ DialogElement::~DialogElement()
{
}

/*virtual*/ bool DialogElement::Load(const rapidjson::Value& elementValue)
{
	if (!elementValue.HasMember("speaker") || !elementValue["speaker"].IsString())
	{
		IMZADI_LOG_ERROR("Expected \"speaker\" member of dialog element.");
		return false;
	}

	this->speaker = elementValue["speaker"].GetString();
	return true;
}

//------------------------------------ DialogData ------------------------------------

DialogBasicElement::DialogBasicElement()
{
}

/*virtual*/ DialogBasicElement::~DialogBasicElement()
{
}

/*virtual*/ bool DialogBasicElement::Load(const rapidjson::Value& elementValue)
{
	if (!DialogElement::Load(elementValue))
		return false;

	if (!elementValue.HasMember("text") || !elementValue["text"].IsString())
	{
		IMZADI_LOG_ERROR("Expected \"text\" member of dialog element.");
		return false;
	}

	this->text = elementValue["text"].GetString();
	return true;
}

//------------------------------------ DialogData ------------------------------------

DialogChoiceElement::DialogChoiceElement()
{
}

/*virtual*/ DialogChoiceElement::~DialogChoiceElement()
{
}

/*virtual*/ bool DialogChoiceElement::Load(const rapidjson::Value& elementValue)
{
	if (!DialogElement::Load(elementValue))
		return false;

	if (!elementValue.HasMember("choice") || !elementValue["choice"].IsArray())
	{
		IMZADI_LOG_ERROR("Expected \"choice\" member of dialog element to exist and be an array.");
		return false;
	}

	this->choiceArray.clear();
	const rapidjson::Value& choiceArrayValue = elementValue["choice"];
	for (int i = 0; i < choiceArrayValue.Size(); i++)
	{
		const rapidjson::Value& choiceValue = choiceArrayValue[i];

		if (!choiceValue.HasMember("text") || !choiceValue["text"].IsString())
		{
			IMZADI_LOG_ERROR("Did not find \"text\" member of choice entry.");
			return false;
		}

		if (!choiceValue.HasMember("sequence") || !choiceValue["sequence"].IsString())
		{
			IMZADI_LOG_ERROR("Did not find \"sequence\" member of choice entry.");
			return false;
		}

		Choice choice;
		choice.text = choiceValue["text"].GetString();
		choice.sequenceName = choiceValue["sequence"].GetString();
		this->choiceArray.push_back(choice);
	}

	return true;
}

//------------------------------------ DialogData ------------------------------------

DialogAcquireElement::DialogAcquireElement()
{
}

/*virtual*/ DialogAcquireElement::~DialogAcquireElement()
{
}

/*virtual*/ bool DialogAcquireElement::Load(const rapidjson::Value& elementValue)
{
	if (!DialogBasicElement::Load(elementValue))
		return false;

	if (!elementValue.HasMember("item") || !elementValue["item"].IsString())
	{
		IMZADI_LOG_ERROR("Expected \"item\" member of dialog element.");
		return false;
	}

	this->acquiredItem = elementValue["item"].GetString();
	return true;
}

//------------------------------------ DialogData ------------------------------------

DialogSequence::DialogSequence()
{
}

/*virtual*/ DialogSequence::~DialogSequence()
{
}