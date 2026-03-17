#include "UClipboardLibrary.h"
#include "HAL/PlatformApplicationMisc.h"

void UClipboardLibrary::CopyToClipboard(const FString& Text)
{
    FPlatformApplicationMisc::ClipboardCopy(*Text);
}