#pragma once

#include "SampleModule.h"

namespace DAVA
{
    SampleModule::SampleModule()
    {
        statusList.emplace_back( eStatus::ES_UNKNOWN );
    }

    void SampleModule::Init()
    {
        statusList.emplace_back( eStatus::ES_INIT );
    }

    void SampleModule::PostInit()
    {
        statusList.emplace_back( eStatus::ES_POST_INIT );
    }

    void SampleModule::Shutdown()
    {
        statusList.emplace_back( eStatus::ES_SHUTDOWN );
    }
}