// sktoolslib - common files for SK tools

// Copyright (C) 2017, 2020 - Stefan Kueng

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
#include <vector>
#include <memory>
#include <comip.h>
#include <comdefsp.h>

_COM_SMARTPTR_TYPEDEF(IUIAnimationStoryboard, __uuidof(IUIAnimationStoryboard));
_COM_SMARTPTR_TYPEDEF(IUIAnimationVariable, __uuidof(IUIAnimationVariable));
_COM_SMARTPTR_TYPEDEF(IUIAnimationManager, __uuidof(IUIAnimationManager));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTransitionLibrary, __uuidof(IUIAnimationTransitionLibrary));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTimer, __uuidof(IUIAnimationTimer));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTimerUpdateHandler, __uuidof(IUIAnimationTimerUpdateHandler));
_COM_SMARTPTR_TYPEDEF(IUIAnimationTransition, __uuidof(IUIAnimationTransition));

class CTimerEventHandler;

class Animator
{
public:
    /// the singleton accessor
    static Animator& Instance();
    /// shuts down the animation manager.
    /// call this *before* COM gets shut down, i.e. before CoUninitialize() or OleUninitialize().
    static void ShutDown();
    /// Disable copying
    Animator(const Animator&) = delete;
    Animator& operator=(const Animator&) = delete;


    IUIAnimationVariablePtr CreateAnimationVariable(double start);
    static INT32 GetIntegerValue(IUIAnimationVariablePtr var);
    static double GetValue(IUIAnimationVariablePtr var);

    IUIAnimationTransitionPtr CreateAccelerateDecelerateTransition(UI_ANIMATION_SECONDS duration, double finalValue, double accelerationRatio = 0.4, double decelerationRatio = 0.4);
    IUIAnimationTransitionPtr CreateSmoothStopTransition(UI_ANIMATION_SECONDS duration, double finalValue);
    IUIAnimationTransitionPtr CreateParabolicTransitionFromAcceleration(double finalValue, double finalVelocity, double acceleration);
    IUIAnimationTransitionPtr CreateCubicTransition(UI_ANIMATION_SECONDS maximumDuration, double finalValue, double finalVelocity);
    IUIAnimationTransitionPtr CreateReversalTransition(UI_ANIMATION_SECONDS duration);
    IUIAnimationTransitionPtr CreateSinusoidalTransitionFromRange(UI_ANIMATION_SECONDS duration, double minimumValue, double maximumValue, UI_ANIMATION_SECONDS period, UI_ANIMATION_SLOPE slope);
    IUIAnimationTransitionPtr CreateSinusoidalTransitionFromVelocity(UI_ANIMATION_SECONDS duration, UI_ANIMATION_SECONDS period);
    IUIAnimationTransitionPtr CreateLinearTransitionFromSpeed(double speed, double finalValue);
    IUIAnimationTransitionPtr CreateLinearTransition(UI_ANIMATION_SECONDS duration, double finalValue);
    IUIAnimationTransitionPtr CreateDiscreteTransition(UI_ANIMATION_SECONDS delay, double finalValue, UI_ANIMATION_SECONDS hold);
    IUIAnimationTransitionPtr CreateConstantTransition(UI_ANIMATION_SECONDS duration);
    IUIAnimationTransitionPtr CreateInstantaneousTransition(double finalValue);

    IUIAnimationStoryboardPtr CreateStoryBoard();

    HRESULT RunStoryBoard(IUIAnimationStoryboardPtr storyBoard, std::function<void()> callback);

    HRESULT AbandonAllStoryBoards();

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
