#include "IWDoorClipboardLibrary.h"
#include "HAL/PlatformApplicationMisc.h"

void UIWDoorClipboardLibrary::CopyToClipboard(const FString& Text)
{
    FPlatformApplicationMisc::ClipboardCopy(*Text);
}