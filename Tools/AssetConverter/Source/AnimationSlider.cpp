#include "AnimationSlider.h"
#include "GamePreview.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "Assets/Animation.h"
#include "Assets/Skeleton.h"

AnimationSlider::AnimationSlider(wxWindow* parent) : wxSlider(parent, wxID_ANY, 0, 0, 24, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS)
{
	this->Bind(wxEVT_SLIDER, &AnimationSlider::OnSliderPositionChanged, this);
}

/*virtual*/ AnimationSlider::~AnimationSlider()
{
}

void AnimationSlider::OnSliderPositionChanged(wxCommandEvent& event)
{
	double sliderValue = (double)this->GetValue();
	double sliderMinValue = (double)this->GetMin();
	double sliderMaxValue = (double)this->GetMax();
	
	double alpha = (sliderValue - sliderMinValue) / (sliderMaxValue - sliderMinValue);

	auto game = (GamePreview*)Imzadi::Game::Get();
	game->SetAnimationMode(GamePreview::AnimationMode::SCRUB);

	Imzadi::AnimatedMeshInstance* animatedMesh = game->GetAnimatingMesh();
	animatedMesh->SetAnimationLocation(alpha);
}