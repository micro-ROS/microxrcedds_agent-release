// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*************************************************************************
 * @file FlowControlExamplePublisher.cpp
 * This file contains the implementation of the publisher functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/rtps/flowcontrol/ThroughputControllerDescriptor.h>

#include <fastrtps/Domain.h>

#include <fastrtps/utils/eClock.h>

#include "FlowControlExamplePublisher.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

FlowControlExamplePublisher::FlowControlExamplePublisher() : mp_participant(nullptr),
                                                             mp_fast_publisher(nullptr),
                                                             mp_slow_publisher(nullptr){}

FlowControlExamplePublisher::~FlowControlExamplePublisher() {	Domain::removeParticipant(mp_participant);}

bool FlowControlExamplePublisher::init()
{
    // Create RTPSParticipant

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_publisher");  //You can put here the name you want

    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant == nullptr)
        return false;

    //Register the type

    Domain::registerType(mp_participant,(TopicDataType*) &myType);

    // Create fast publisher, which has no controller of its own.
    PublisherAttributes WparamFast;
    WparamFast.topic.topicKind = NO_KEY;
    WparamFast.topic.topicDataType = myType.getName();  //This type MUST be registered
    WparamFast.topic.topicName = "FlowControlExamplePubSubTopic";
    WparamFast.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;

    mp_fast_publisher = Domain::createPublisher(mp_participant,WparamFast,(PublisherListener*)&m_listener);
    if(mp_fast_publisher == nullptr)
        return false;
    std::cout << "Fast publisher created, waiting for Subscribers." << std::endl;

    // Create slow publisher, with its own controller
    PublisherAttributes WparamSlow;
    WparamSlow.topic.topicKind = NO_KEY;
    WparamSlow.topic.topicDataType = myType.getName();  //This type MUST be registered
    WparamSlow.topic.topicName = "FlowControlExamplePubSubTopic";
    WparamSlow.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;

    // This controller allows 300kb per second.
    ThroughputControllerDescriptor slowPublisherThroughputController{300000, 1000};
    WparamSlow.throughputController = slowPublisherThroughputController;

    mp_slow_publisher = Domain::createPublisher(mp_participant,WparamSlow,(PublisherListener*)&m_listener);
    if(mp_slow_publisher == nullptr)
        return false;
    std::cout << "Slow publisher created, waiting for Subscribers." << std::endl;
    return true;
}

void FlowControlExamplePublisher::PubListener::onPublicationMatched(Publisher*, MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched" << std::endl;
    }
}

void FlowControlExamplePublisher::run()
{
    while(m_listener.n_matched == 0)
    {
        eClock::my_sleep(250); // Sleep 250 ms
    }

    // Publication code

    FlowControlExample st;

    /* Initialize your structure here */

    int msgsent_fast = 0;
    int msgsent_slow = 0;
    char ch;
    std::cout << "Flow Control example." << std::endl;
    std::cout << "Press \"f\" to send a sample through the fast writer, which has unlimited bandwidth" << std::endl;
    std::cout << "Press \"s\" to send a sample through the slow writer, which is also limited by its own Flow Controller" << std::endl;
    std::cout << "Press \"q\" quit" << std::endl;
    while(std::cin >> ch)
    {
        if(ch == 'f')
        {
            st.wasFast(true);
            mp_fast_publisher->write(&st);  ++msgsent_fast;
            std::cout << "Sending sample, count=" << msgsent_fast << " through the fast writer. Send another sample? (f-fast,s-slow,q-quit): ";
        }
        else if(ch == 's')
        {
            st.wasFast(false);
            mp_slow_publisher->write(&st);  ++msgsent_slow;
            std::cout << "Sending sample, count=" << msgsent_slow << " through the slow writer. Send another sample? (f-fast,s-slow,q-quit): ";
        }
        else if (ch == 'q')
        {
            std::cout << "Finishing Flow Control example" << std::endl;
            break;
        }
        else
        {
            std::cout << "Command " << ch << " not recognized, please enter \"f/s/q\":";
        }

    }
}
