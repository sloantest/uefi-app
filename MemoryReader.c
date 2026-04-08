/** @file
  Simple UEFI Memory Reader - Displays system memory map
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                 Status;
  UINTN                      MemoryMapSize = 0;
  EFI_MEMORY_DESCRIPTOR      *MemoryMap = NULL;
  UINTN                      MapKey;
  UINTN                      DescriptorSize;
  UINT32                     DescriptorVersion;
  UINTN                      Index;
  UINTN                      NumberOfEntries;
  
  Print(L"========================================\n");
  Print(L"UEFI Memory Reader\n");
  Print(L"========================================\n\n");
  
  // First call to get required buffer size
  Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"ERROR: GetMemoryMap failed: %r\n", Status);
    return Status;
  }
  
  // Add extra space for safety
  MemoryMapSize += 4096;
  
  // Allocate buffer
  MemoryMap = AllocatePool(MemoryMapSize);
  if (MemoryMap == NULL) {
    Print(L"ERROR: Failed to allocate memory\n");
    return EFI_OUT_OF_RESOURCES;
  }
  
  // Get the actual memory map
  Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Second GetMemoryMap call failed: %r\n", Status);
    FreePool(MemoryMap);
    return Status;
  }
  
  NumberOfEntries = MemoryMapSize / DescriptorSize;
  
  Print(L"Memory Map contains %d entries\n", NumberOfEntries);
  Print(L"Descriptor size: %d bytes\n\n", DescriptorSize);
  Print(L"Type                Start Address        Pages\n");
  Print(L"-----------------------------------------------\n");
  
  for (Index = 0; Index < NumberOfEntries && Index < 30; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + (Index * DescriptorSize));
    
    switch (Desc->Type) {
      case EfiLoaderCode:           Print(L"LoaderCode          "); break;
      case EfiLoaderData:           Print(L"LoaderData          "); break;
      case EfiBootServicesCode:     Print(L"BootServicesCode    "); break;
      case EfiBootServicesData:     Print(L"BootServicesData    "); break;
      case EfiRuntimeServicesCode:  Print(L"RuntimeServicesCode "); break;
      case EfiRuntimeServicesData:  Print(L"RuntimeServicesData "); break;
      case EfiConventionalMemory:   Print(L"Conventional        "); break;
      case EfiReservedMemoryType:   Print(L"Reserved            "); break;
      case EfiACPIReclaimMemory:    Print(L"ACPI Reclaim        "); break;
      case EfiACPIMemoryNVS:        Print(L"ACPI NVS            "); break;
      default:                      Print(L"Other               "); break;
    }
    
    Print(L"  0x%016lx     0x%08lx pages\n", Desc->PhysicalStart, Desc->NumberOfPages);
  }
  
  Print(L"\nMemory Reader completed successfully!\n");
  Print(L"Press any key to exit...\n");
  
  // Wait for a key press so you can see the output
  SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
  UINTN Index2;
  EFI_INPUT_KEY Key;
  gBS->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &Index2);
  SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
  
  FreePool(MemoryMap);
  return EFI_SUCCESS;
}
