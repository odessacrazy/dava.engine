#pragma once

#include "Base/BaseTypes.h"

#include "TArc/WindowSubSystem/UI.h"

class QMainWindow;
namespace DAVA
{
namespace TArc
{
class PropertiesItem;

struct KeyBindableAction
{
    QString blockName;
    QString actionName;
    Qt::ShortcutContext context = Qt::WidgetShortcut;
    QList<QKeySequence> sequences;
    QPointer<QAction> action;
};

class UIManager final : public UI
{
public:
    class Delegate
    {
    public:
        virtual bool WindowCloseRequested(const WindowKey& key) = 0;
        virtual void OnWindowClosed(const WindowKey& key) = 0;
    };
    UIManager(Delegate* delegate, PropertiesItem&& holder);
    ~UIManager();

    void InitializationFinished();

    void DeclareToolbar(const WindowKey& windowKey, const ActionPlacementInfo& toogleToolbarVisibility, const QString& toolbarName) override;

    void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) override;
    void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) override;
    void RemoveAction(const WindowKey& windowKey, const ActionPlacementInfo& placement) override;

    void ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration = 0) override;
    void ClearMessage(const WindowKey& windowKey) override;
    int ShowModalDialog(const WindowKey& windowKey, QDialog* dialog) override;
    ModalMessageParams::Button ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params) override;
    void ShowNotification(const WindowKey& windowKey, const NotificationParams& params) override;

    QString GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params) override;
    QString GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params) override;
    QString GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params) override;

    std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params = WaitDialogParams()) override;
    bool HasActiveWaitDalogues() const override;
    DAVA_DEPRECATED(QWidget* GetWindow(const WindowKey& windowKey) override);

    DAVA_DEPRECATED(void InjectWindow(const WindowKey& windowKey, QMainWindow* window) override);
    void ModuleDestroyed(ClientModule* module);

    const Vector<KeyBindableAction>& GetKeyBindableActions() const;

protected:
    void SetCurrentModule(ClientModule* module) override;

private:
    void RegisterAction(QAction* action);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
} // namespace TArc
} // namespace DAVA
