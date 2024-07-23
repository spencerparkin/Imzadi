#include "DialogData.h"
#include "GameApp.h"
#include "Scene.h"
#include "RenderObjects/TextRenderObject.h"
#include "Entity.h"
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

//------------------------------------ DialogElement ------------------------------------

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

//------------------------------------ DialogBasicElement ------------------------------------

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

/*virtual*/ bool DialogBasicElement::Setup()
{
	Imzadi::Reference<Imzadi::Entity> foundEntity;
	if (!Imzadi::Game::Get()->FindEntityByName(this->speaker, foundEntity))
		return false;

	Imzadi::Transform transform;
	foundEntity->GetTransform(transform);
	transform.matrix.SetIdentity();
	transform.matrix.SetUniformScale(20.0);

	uint32_t flags =
		Imzadi::TextRenderObject::Flag::ALWAYS_FACING_CAMERA |
		Imzadi::TextRenderObject::Flag::ALWAYS_ON_TOP |
		Imzadi::TextRenderObject::Flag::CENTER_JUSTIFY |
		Imzadi::TextRenderObject::Flag::OPAQUE_BACKGROUND |
		Imzadi::TextRenderObject::Flag::MULTI_LINE;

	auto textRenderObject = new Imzadi::TextRenderObject();
	textRenderObject->SetFont("Roboto_Regular");
	textRenderObject->SetFlags(flags);
	textRenderObject->SetText(std::format("{}: {} (Press \"A\".)", this->speaker.c_str(), this->text.c_str()));
	textRenderObject->SetForegroundColor(Imzadi::Vector3(1.0, 1.0, 1.0));
	textRenderObject->SetBackgroundColor(Imzadi::Vector3(0.0, 0.0, 0.0));
	textRenderObject->SetTransform(transform);
	this->sceneObjName = Imzadi::Game::Get()->GetScene()->AddRenderObject(textRenderObject);

	return true;
}

/*virtual*/ bool DialogBasicElement::Shutdown()
{
	Imzadi::Game::Get()->GetScene()->RemoveRenderObject(this->sceneObjName);
	return true;
}

/*virtual*/ bool DialogBasicElement::Tick(std::string& nextSequence, int& nextSequencePosition)
{
	Imzadi::Controller* controller = Imzadi::Game::Get()->GetController("DialogSystem");
	if (!controller)
		return false;

	if (controller->ButtonPressed(XINPUT_GAMEPAD_A, true))
	{
		nextSequencePosition++;
		return false;
	}

	return true;
}

//------------------------------------ DialogChoiceElement ------------------------------------

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

/*virtual*/ bool DialogChoiceElement::Setup()
{
	return false;
}

/*virtual*/ bool DialogChoiceElement::Shutdown()
{
	return false;
}

/*virtual*/ bool DialogChoiceElement::Tick(std::string& nextSequence, int& nextSequencePosition)
{
	return false;
}

//------------------------------------ DialogAcquireElement ------------------------------------

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

/*virtual*/ bool DialogAcquireElement::Setup()
{
	return false;
}

/*virtual*/ bool DialogAcquireElement::Shutdown()
{
	return false;
}

/*virtual*/ bool DialogAcquireElement::Tick(std::string& nextSequence, int& nextSequencePosition)
{
	return false;
}

//------------------------------------ DialogSequence ------------------------------------

DialogSequence::DialogSequence()
{
}

/*virtual*/ DialogSequence::~DialogSequence()
{
}