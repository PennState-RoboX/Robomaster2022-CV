def machine_type():
    import platform

    # Get the machine type and processor information
    machine = platform.machine()
    processor = platform.processor()

    print(machine)
    print(processor)

#     # Determine the CPU type
#     if "x86" in machine or "x86" in processor:
#         if "Intel" in processor:
#             cpu_type = "Intel CPU"
#         elif "AMD" in processor:
#             cpu_type = "AMD CPU"
#         else:
#             cpu_type = "x86 architecture (Unknown manufacturer)"
#     elif "arm" in machine.lower() or "arm" in processor.lower():
#         cpu_type = "ARM architecture"
#     else:
#         cpu_type = "Unknown architecture"
    
#     return cpu_type
machine_type()
# print(device)
