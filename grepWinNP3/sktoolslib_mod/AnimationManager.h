// sktoolslib - common files for SK tools

// Copyright (C) 2017, 2020-2021 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


/**
 * \brief helper class for the Windows Animation Manager.
 * Provides convenience methods to use the Windows Animation Manager using
 * timer events.
 * 
 * Example to animate transparency:
 * \code
 * // create the animation variable for the alpha value to animate.
 * // Note: this variable can be a temp variable, but then subsequent
 * // animations won't stop previous animations of the variable.
 * CMyWindow::CMyWindow()
 * {
 *      // m_AnimVarAlpha is a member variable of type IUIAnimationVariablePtr
 *      m_AnimVarAlpha = Animator::Instance().CreateAnimationVariable(0);
 * }
 * void MakeVisible()
 * {
 *       auto transAlpha = Animator::Instance().CreateLinearTransition(0.3, 255);
 *       auto storyBoard = Animator::Instance().CreateStoryBoard();
 *       storyBoard->AddTransition(m_AnimVarAlpha, transAlpha);
 *       Animator::Instance().RunStoryBoard(storyBoard, [this]()
 *       {
 *           SetTransparency((BYTE)Animator::GetIntegerValue(m_AnimVarAlpha));
 *       });
 * }
 *
 * void MakeInvisible()
 * {
 *       auto transAlpha = Animator::Instance().CreateLinearTransition(0.5, 0);
 *       auto storyBoard = Animator::Instance().CreateStoryBoard();
 *       storyBoard->AddTransition(m_AnimVarAlpha, transAlpha);
 *       Animator::Instance().RunStoryBoard(storyBoard, [this]()
 *       {
 *           SetTransparency((BYTE)Animator::GetIntegerValue(m_AnimVarAlpha));
 *       });
 * }
 */

#pragma once
#include <UIAnimation.h>
#include <functional>
#include <memory>
#include <comip.h>

_COM_SMARTPTR_TYPEDEF(IUIAnimationStoryboard, __uuidof(IUIAnimationStoryboard));
_COM_SMARTPTR_TYPEDEF(IUIAnimationVariable, __uuidof(IUIAnimationVariable));
_COM_SMARTPTR_TYPEDEF(IUIAnimationManager, __uuidof(IUIAnimationManager));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTransitionLibrary, __uuidof(IUIAnimationTransitionLibrary));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTimer, __uuidof(IUIAnimationTimer));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTimerUpdateHandler, __uuidof(IUIAnimationTimerUpdateHandler));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTransition, __uuidof(IUIAnimationTransition));

class CTimerEventHandler;

class AnimationVariable
{
public:
    IUIAnimationVariablePtr m_animVar;
    double m_defaultValue = 0.0;

};

class Animator
{
public:
    /// the singleton accessor
    static Animator& Instance();
    /// shuts down the animation manager.
    /// call this *before* COM gets shut down, i.e. before CoUninitialize() or OleUninitialize().
    static void ShutDown();
    static bool IsInstanceActive();
    /// Disable copying
    Animator(const Animator&) = delete;
    Animator& operator=(const Animator&) = delete;


    AnimationVariable CreateAnimationVariable(double start, double defValue) const;
    static INT32      GetIntegerValue(AnimationVariable& var);
    static double     GetValue(AnimationVariable& var);

    IUIAnimationTransitionPtr CreateAccelerateDecelerateTransition(AnimationVariable& var, UI_ANIMATION_SECONDS duration, double finalValue, double accelerationRatio = 0.4, double decelerationRatio = 0.4) const;
    IUIAnimationTransitionPtr CreateSmoothStopTransition(AnimationVariable& var, UI_ANIMATION_SECONDS duration, double finalValue) const;
    IUIAnimationTransitionPtr CreateParabolicTransitionFromAcceleration(AnimationVariable& var, double finalValue, double finalVelocity, double acceleration) const;
    IUIAnimationTransitionPtr CreateCubicTransition(AnimationVariable& var, UI_ANIMATION_SECONDS maximumDuration, double finalValue, double finalVelocity) const;
    IUIAnimationTransitionPtr CreateReversalTransition(UI_ANIMATION_SECONDS duration) const;
    IUIAnimationTransitionPtr CreateSinusoidalTransitionFromRange(UI_ANIMATION_SECONDS duration, double minimumValue, double maximumValue, UI_ANIMATION_SECONDS period, UI_ANIMATION_SLOPE slope) const;
    IUIAnimationTransitionPtr CreateSinusoidalTransitionFromVelocity(UI_ANIMATION_SECONDS duration, UI_ANIMATION_SECONDS period) const;
    IUIAnimationTransitionPtr CreateLinearTransitionFromSpeed(AnimationVariable& var, double speed, double finalValue) const;
    IUIAnimationTransitionPtr CreateLinearTransition(AnimationVariable& var, UI_ANIMATION_SECONDS duration, double finalValue) const;
    IUIAnimationTransitionPtr CreateDiscreteTransition(AnimationVariable& var, UI_ANIMATION_SECONDS delay, double finalValue, UI_ANIMATION_SECONDS hold) const;
    IUIAnimationTransitionPtr CreateConstantTransition(UI_ANIMATION_SECONDS duration) const;
    IUIAnimationTransitionPtr CreateInstantaneousTransition(AnimationVariable& var, double finalValue) const;

    IUIAnimationStoryboardPtr CreateStoryBoard() const;

    HRESULT RunStoryBoard(IUIAnimationStoryboardPtr storyBoard, std::function<void()> callback) const;

    HRESULT AbandonAllStoryBoards() const;

    virtual ~Animator();
private:
    Animator();

    static std::unique_ptr<Animator> instance;
    /// The holder of the UIAnimationManager
    IUIAnimationManagerPtr pAnimMgr;
    /// The holder of the UIAnimationTimer
    IUIAnimationTimerPtr pAnimTmr;
    /// The holder of the UITransitionLibrary
    IUIAnimationTransitionLibraryPtr pTransLib;
    /// the timer callback object
    CTimerEventHandler * timerEventHandler;
};
