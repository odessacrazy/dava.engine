#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Android/JNIBridge.h"

#include "Math/Rect.h"
#include "Functional/Function.h"

namespace DAVA
{
class Window;
class UITextField;
class UITextFieldDelegate;

class Color;

class TextFieldControlImpl final : public std::enable_shared_from_this<TextFieldControlImpl>
{
public:
    TextFieldControlImpl(Window& w, UITextField& uiTextField);
    ~TextFieldControlImpl();

    void Initialize();
    void OwnerIsDying();

    void SetVisible(bool visible);
    void SetIsPassword(bool password);
    void SetMaxLength(int32 value);

    void OpenKeyboard();
    void CloseKeyboard();

    void UpdateRect(const Rect& rect);

    void SetText(const WideString& text);
    void GetText(WideString& text) const;

    void SetTextColor(const Color& color);
    void SetTextAlign(int32 align);
    int32 GetTextAlign() const;
    void SetTextUseRtlAlign(bool useRtlAlign);
    bool GetTextUseRtlAlign() const;

    void SetFontSize(float32 virtualFontSize);

    void SetDelegate(UITextFieldDelegate* textFieldDelegate);

    void SetMultiline(bool enable);

    void SetInputEnabled(bool enable);

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

    void SetAutoCapitalizationType(int32 value);
    void SetAutoCorrectionType(int32 value);
    void SetSpellCheckingType(int32 value);
    void SetKeyboardAppearanceType(int32 value);
    void SetKeyboardType(int32 value);
    void SetReturnKeyType(int32 value);
    void SetEnableReturnKeyAutomatically(bool value);

    uint32 GetCursorPos() const;
    void SetCursorPos(uint32 pos);

    void nativeOnFocusChange(JNIEnv* env, jboolean hasFocus);
    void nativeOnKeyboardShown(JNIEnv* env, jint x, jint y, jint w, jint h);
    void nativeOnEnterPressed(JNIEnv* env);
    jboolean nativeOnKeyPressed(JNIEnv* env, jint replacementStart, jint replacementLength, jstring replaceWith);
    void nativeOnTextChanged(JNIEnv* env, jstring newText, jboolean programmaticTextChange);
    void nativeOnTextureReady(JNIEnv* env, jintArray pixels, jint w, jint h);

private:
    void OnFocusChanged(bool hasFocus);
    void OnKeyboardShown(const Rect& keyboardRect);
    void OnEnterPressed();
    bool OnKeyPressed(int32 replacementStart, int32 replacementLength, WideString& replaceWith);
    void OnTextChanged(const WideString& newText, bool programmaticTextChange);

private:
    Window& window;
    UITextField* uiTextField = nullptr;
    UITextFieldDelegate* uiTextFieldDelegate = nullptr;
    jobject javaTextField = nullptr;

    Rect controlRect;
    WideString curText;
    bool multiline = false;
    bool textRtlAlign = false;
    int32 textAlign = 0;
    int32 maxTextLength = -1;

    std::unique_ptr<JNI::JavaClass> textFieldJavaClass;
    Function<void(jobject)> release;
    Function<void(jobject, jboolean)> setVisible;
    Function<void(jobject, jboolean)> setIsPassword;
    Function<void(jobject, jint)> setMaxLength;
    Function<void(jobject)> openKeyboard;
    Function<void(jobject)> closeKeyboard;
    Function<void(jobject, jfloat, jfloat, jfloat, jfloat)> setRect;
    Function<void(jobject, jstring)> setText;
    Function<void(jobject, jint, jint, jint, jint)> setTextColor;
    Function<void(jobject, jint)> setTextAlign;
    Function<void(jobject, jboolean)> setTextUseRtlAlign;
    Function<void(jobject, jfloat)> setFontSize;
    Function<void(jobject, jboolean)> setMultiline;
    Function<void(jobject, jboolean)> setInputEnabled;
    Function<void(jobject, jint)> setAutoCapitalizationType;
    Function<void(jobject, jint)> setAutoCorrectionType;
    Function<void(jobject, jint)> setSpellCheckingType;
    Function<void(jobject, jint)> setKeyboardAppearanceType;
    Function<void(jobject, jint)> setKeyboardType;
    Function<void(jobject, jint)> setReturnKeyType;
    Function<void(jobject, jboolean)> setEnableReturnKeyAutomatically;
    Function<jint(jobject)> getCursorPos;
    Function<void(jobject, jint)> setCursorPos;
    Function<void(jobject)> update;
};

inline void TextFieldControlImpl::SetRenderToTexture(bool /*value*/)
{
    // Do nothing as single line text field always is painted into texture
    // Multiline text field is never rendered to texture
}

inline bool TextFieldControlImpl::IsRenderToTexture() const
{
    return !multiline;
}

inline int32 TextFieldControlImpl::GetTextAlign() const
{
    return textAlign;
}

inline bool TextFieldControlImpl::GetTextUseRtlAlign() const
{
    return textRtlAlign;
}

} // namespace DAVA 

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
