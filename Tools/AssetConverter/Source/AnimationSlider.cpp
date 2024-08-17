#include "AnimationSlider.h"
#include "GamePreview.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "Assets/Animation.h"
#include "Assets/Skeleton.h"

AnimationSlider::AnimationSlider(wxWindow* parent) : wxSlider(parent, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS)
{
	this->Bind(wxEVT_SLIDER, &AnimationSlider::OnSliderPositionChanged, this);
}

/*virtual*/ AnimationSlider::~AnimationSlider()
{
}

void AnimationSlider::OnSliderPositionChanged(wxCommandEvent& event)
{
	auto game = (GamePreview*)Imzadi::Game::Get();
	Imzadi::AnimatedMeshInstance* animatedMesh = game->GetAnimatingMesh();
	if (!animatedMesh)
		return;

#if 1
	game->SetAnimationMode(GamePreview::AnimationMode::SCRUB_INTERPOLATE);
#else
	game->SetAnimationMode(GamePreview::AnimationMode::SCRUB_KEYFRAMES);
#endif

	switch (game->GetAnimationMode())
	{
		case GamePreview::AnimationMode::SCRUB_INTERPOLATE:
		{
			double sliderValue = (double)this->GetValue();
			double sliderMinValue = (double)this->GetMin();
			double sliderMaxValue = (double)this->GetMax();
			double alpha = (sliderValue - sliderMinValue) / (sliderMaxValue - sliderMinValue);
			animatedMesh->SetAnimationLocation(alpha);
			break;
		}
		case GamePreview::AnimationMode::SCRUB_KEYFRAMES:
		{
			int i = this->GetValue();
			animatedMesh->SetAnimationLocation(i);
			break;
		}
	}
}