#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Qt/QtIcon.h>

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class UIScriptSystem;
}

class ScriptControlModule : public DAVA::TArc::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(ScriptControlModule, DAVA::TArc::ClientModule);

protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;
    void PostInit() override;

private:
    bool IsEnabled() const;
    const QIcon& GetPauseButtonIcon();
    void PlayPause();
    DAVA::UIScriptSystem* GetScriptSystem() const;

    const DAVA::String pauseButtonHint = "Play/Pause Script processing";
};
