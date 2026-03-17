#include "VelvetActivatableWidget.h"

void UVelvetActivatableWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
    OnWidgetActivated();
}

void UVelvetActivatableWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
    OnWidgetDeactivated();
}