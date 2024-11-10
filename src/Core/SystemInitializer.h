/*
 * This file is auto-generated. Do not modify it manually.
 * Any changes made will be overwritten the next time the script is run.
 * <Core/ServiceLocator.h> must be included before calling the macro.
 */


#ifndef SYSTEM_INITIALIZER_H
#define SYSTEM_INITIALIZER_H

#include <Systems/CameraSystem.h>

#define INITIALIZE_SYSTEMS()                   \
    static CameraSystem CameraSystem_instance; \
                                               \
    ServiceLocator::get_system_manager()       \
        .add(CameraSystem_instance)            \
        .register_events();

#endif // SYSTEM_INITIALIZER_H