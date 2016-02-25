/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "FileSystem2Test.h"
#include "FileSystem/FileSystem2.h"

FileSystem2Test::FileSystem2Test()
{

}

void FileSystem2Test::StartTest(BaseObject*, void*, void*)
{
    using namespace DAVA;
    Path path("~res:/TestData");

    if (FileSystem2::IsFile(path))
    {
        Logger::Info("path:%s is file", path.ToStringUtf8().c_str());
    }

    if (FileSystem2::IsDirectory(path))
    {
        Logger::Info("path:%s is directory", path.ToStringUtf8().c_str());
    }

    Path dirName("MovieTest");
    path += dirName;
    Logger::Info("path: %s", path.ToStringUtf8().c_str());

    Path fileName("")
}

void FileSystem2Test::LoadResources()
{
    using namespace DAVA;
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    ScopedPtr<UIButton> resetButton(new UIButton(Rect(420, 30, 200, 30)));
    resetButton->SetDebugDraw(true);
    resetButton->SetStateFont(0xFF, font);
    resetButton->SetStateFontColor(0xFF, Color::White);
    resetButton->SetStateText(0xFF, L"Generate Floating point exception");
    resetButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FileSystem2Test::StartTest));
    AddControl(resetButton.get());
}
