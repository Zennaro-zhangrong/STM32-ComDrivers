/** @file display_transfer_service.c @brief 满队列返回失败；先保存owner再移动描述符。 */
#include "display_transfer_service.h"
static void Finish(DisplayTransferService *service, bool success)
{
    void *owner = service->pending[0].owner;
    --service->count;
    if (service->count != 0) service->pending[0] = service->pending[1];
    service->active = false;
    service->complete(owner, success);
}
void DisplayTransfer_Process(DisplayTransferService *service)
{
    if (service->dispatching) return;
    service->dispatching = true;
    for (unsigned attempts = 0; attempts < 2 && service->count != 0 && !service->active; ++attempts) {
        service->active = true;
        if (!service->start(service->context, &service->pending[0])) Finish(service, false);
    }
    service->dispatching = false;
}
bool DisplayTransfer_Enqueue(DisplayTransferService *service, void *owner,
    uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, const uint8_t *pixels)
{
    if (service == NULL || service->count >= 2 || pixels == NULL || service->start == NULL ||
        service->complete == NULL || right < left || bottom < top) return false;
    uint64_t bytes = ((uint64_t)right - left + 1) * ((uint64_t)bottom - top + 1) * 2;
    if (bytes > SIZE_MAX) return false;
    service->pending[service->count++] = (DisplayTransfer){left,top,right,bottom,pixels,(size_t)bytes,owner};
    DisplayTransfer_Process(service);
    return true;
}
static void Notify(DisplayTransferService *service, bool success)
{
    if (service == NULL || service->dispatching || !service->active || service->count == 0) return;
    service->dispatching = true;
    Finish(service, success);
    service->dispatching = false;
    DisplayTransfer_Process(service);
}
void DisplayTransfer_OnComplete(DisplayTransferService *service) { Notify(service, true); }
void DisplayTransfer_OnError(DisplayTransferService *service) { Notify(service, false); }
