//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "ObstacleLoss.h" // TODO: delme
#include "Radio.h"
#include "RadioMedium.h"
#include "RadioControlInfo_m.h"
#include "IMACFrame.h"
#include "NotifierConsts.h"
#include "ModuleAccess.h"
#include "IInterfaceTable.h"

namespace inet {

using namespace physicallayer;

Define_Module(RadioMedium);

RadioMedium::RadioMedium() :
    propagation(NULL),
    pathLoss(NULL),
    attenuation(NULL),
    backgroundNoise(NULL),
    maxTransmissionPower(W(sNaN)),
    minInterferencePower(W(sNaN)),
    minReceptionPower(W(sNaN)),
    maxAntennaGain(sNaN),
    minInterferenceTime(sNaN),
    maxTransmissionDuration(sNaN),
    maxCommunicationRange(m(sNaN)),
    maxInterferenceRange(m(sNaN)),
    rangeFilter(RANGE_FILTER_ANYWHERE),
    radioModeFilter(false),
    listeningFilter(false),
    macAddressFilter(false),
    recordCommunication(false),
    removeNonInterferingTransmissionsTimer(NULL),
    baseRadioId(0),
    baseTransmissionId(0),
    neighborCache(NULL),
    transmissionCount(0),
    sendCount(0),
    receptionComputationCount(0),
    receptionDecisionComputationCount(0),
    listeningDecisionComputationCount(0),
    cacheReceptionGetCount(0),
    cacheReceptionHitCount(0),
    cacheDecisionGetCount(0),
    cacheDecisionHitCount(0)
{
}

RadioMedium::RadioMedium(const IPropagation *propagation, const IAttenuation *attenuation, const IBackgroundNoise *backgroundNoise, const simtime_t minInterferenceTime, const simtime_t maxTransmissionDuration, m maxCommunicationRange, m maxInterferenceRange) :
    propagation(propagation),
    pathLoss(NULL),
    attenuation(attenuation),
    backgroundNoise(backgroundNoise),
    maxTransmissionPower(W(sNaN)),
    minInterferencePower(W(sNaN)),
    minReceptionPower(W(sNaN)),
    maxAntennaGain(sNaN),
    minInterferenceTime(minInterferenceTime),
    maxTransmissionDuration(maxTransmissionDuration),
    maxCommunicationRange(m(maxCommunicationRange)),
    maxInterferenceRange(m(maxInterferenceRange)),
    rangeFilter(RANGE_FILTER_ANYWHERE),
    radioModeFilter(false),
    listeningFilter(false),
    macAddressFilter(false),
    recordCommunication(false),
    removeNonInterferingTransmissionsTimer(NULL),
    baseRadioId(0),
    baseTransmissionId(0),
    neighborCache(NULL),
    transmissionCount(0),
    sendCount(0),
    receptionComputationCount(0),
    receptionDecisionComputationCount(0),
    listeningDecisionComputationCount(0),
    cacheReceptionGetCount(0),
    cacheReceptionHitCount(0),
    cacheDecisionGetCount(0),
    cacheDecisionHitCount(0)
{
}

RadioMedium::~RadioMedium()
{
    delete backgroundNoise;
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
        delete *it;
    cancelAndDelete(removeNonInterferingTransmissionsTimer);
    for (std::vector<TransmissionCacheEntry>::const_iterator it = cache.begin(); it != cache.end(); it++)
    {
        const TransmissionCacheEntry &transmissionCacheEntry = *it;
        delete transmissionCacheEntry.frame;
        const std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        if (receptionCacheEntries)
        {
            for (std::vector<ReceptionCacheEntry>::const_iterator jt = receptionCacheEntries->begin(); jt != receptionCacheEntries->end(); jt++)
            {
                const ReceptionCacheEntry &cacheEntry = *jt;
                delete cacheEntry.arrival;
                delete cacheEntry.listening;
                delete cacheEntry.reception;
                delete cacheEntry.decision;
            }
            delete receptionCacheEntries;
        }
    }
    if (recordCommunication)
        communicationLog.close();
}

void RadioMedium::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        // initialize parameters
        propagation = check_and_cast<IPropagation *>(getSubmodule("propagation"));
        pathLoss = check_and_cast<IPathLoss *>(getSubmodule("pathLoss"));
        obstacleLoss = new ObstacleLoss(); // TODO: dynamic_cast<IObstacleLoss *>(getSubmodule("obstacleLoss"));
        attenuation = check_and_cast<IAttenuation *>(getSubmodule("attenuation"));
        backgroundNoise = dynamic_cast<IBackgroundNoise *>(getSubmodule("backgroundNoise"));
        const char *rangeFilterString = par("rangeFilter");
        if (!strcmp(rangeFilterString, ""))
            rangeFilter = RANGE_FILTER_ANYWHERE;
        else if (!strcmp(rangeFilterString, "interferenceRange"))
            rangeFilter = RANGE_FILTER_INTERFERENCE_RANGE;
        else if (!strcmp(rangeFilterString, "communicationRange"))
            rangeFilter = RANGE_FILTER_COMMUNICATION_RANGE;
        else
            throw cRuntimeError("Unknown range filter: '%s'", rangeFilter);
        radioModeFilter = par("radioModeFilter");
        listeningFilter = par("listeningFilter");
        macAddressFilter = par("macAddressFilter");
        // initialize timers
        removeNonInterferingTransmissionsTimer = new cMessage("removeNonInterferingTransmissions");
        // initialize logging
        recordCommunication = par("recordCommunication");
        if (recordCommunication)
            communicationLog.open(ev.getConfig()->substituteVariables("${resultdir}/${configname}-${runnumber}.tlog"));
        // TODO: create cache based on parameter?
        // e.g. neighborCache = new XXXNeighborCache();
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        updateLimits();
    }
    else if (stage == INITSTAGE_LAST)
    {
        EV_DEBUG << "Radio medium initialized"
                 << ", maximum transmission power = " << maxTransmissionPower
                 << ", minimum interference power = " << minInterferencePower
                 << ", minimum reception power = " << minReceptionPower
                 << ", maximum antenna gain = " << maxAntennaGain
                 << ", minimum interference time = " << minInterferenceTime << " s"
                 << ", maximum transmission duration = " << maxTransmissionDuration << " s"
                 << ", maximum communication range = " << maxCommunicationRange
                 << ", maximum interference range = " << maxInterferenceRange
                 << ", " << propagation << ", " << attenuation;
        if (backgroundNoise)
            EV_DEBUG << ", " << backgroundNoise;
        else
            EV_DEBUG << ", no background noise";
        EV_DEBUG << endl;
    }
}


void RadioMedium::finish()
{
    double receptionCacheHitPercentage = 100 * (double)cacheReceptionHitCount / (double)cacheReceptionGetCount;
    double decisionCacheHitPercentage = 100 * (double)cacheDecisionHitCount / (double)cacheDecisionGetCount;
    EV_INFO << "Radio medium transmission count = " << transmissionCount << endl;
    EV_INFO << "Radio medium radio frame send count = " << sendCount << endl;
    EV_INFO << "Radio medium reception computation count = " << receptionComputationCount << endl;
    EV_INFO << "Radio medium reception decision computation count = " << receptionDecisionComputationCount << endl;
    EV_INFO << "Radio medium listening decision computation count = " << listeningDecisionComputationCount << endl;
    EV_INFO << "Radio medium reception cache hit = " << receptionCacheHitPercentage << " %" << endl;
    EV_INFO << "Radio medium reception decision cache hit = " << decisionCacheHitPercentage << " %" << endl;
    recordScalar("Radio medium transmission count", transmissionCount);
    recordScalar("Radio medium radio frame send count", sendCount);
    recordScalar("Radio medium reception computation count", receptionComputationCount);
    recordScalar("Radio medium reception decision computation count", receptionDecisionComputationCount);
    recordScalar("Radio medium listening decision computation count", listeningDecisionComputationCount);
    recordScalar("Radio medium reception cache hit", receptionCacheHitPercentage, "%");
    recordScalar("Radio medium reception decision cache hit", decisionCacheHitPercentage, "%");
}

void RadioMedium::handleMessage(cMessage *message)
{
    if (message == removeNonInterferingTransmissionsTimer)
        removeNonInterferingTransmissions();
    else
        throw cRuntimeError("Unknown message");
}

RadioMedium::TransmissionCacheEntry *RadioMedium::getTransmissionCacheEntry(const ITransmission *transmission) const
{
    size_t transmissionIndex = transmission->getId() - baseTransmissionId;
    if (transmissionIndex < 0)
        return NULL;
    else
    {
        if (transmissionIndex >= cache.size())
            cache.resize(transmissionIndex + 1);
        TransmissionCacheEntry &transmissionCacheEntry = cache[transmissionIndex];
        if (!transmissionCacheEntry.receptionCacheEntries)
            transmissionCacheEntry.receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radios.size());
        return &transmissionCacheEntry;
    }
}

RadioMedium::ReceptionCacheEntry *RadioMedium::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission) const
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (!transmissionCacheEntry)
        return NULL;
    else
    {
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry->receptionCacheEntries;
        size_t radioIndex = radio->getId() - baseRadioId;
        if (radioIndex < 0)
            return NULL;
        else
        {
            if (radioIndex >= receptionCacheEntries->size())
                receptionCacheEntries->resize(radioIndex + 1);
            return &(*receptionCacheEntries)[radioIndex];
        }
    }
}

const IArrival *RadioMedium::getCachedArrival(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry  ? cacheEntry->arrival : NULL;
}

void RadioMedium::setCachedArrival(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->arrival = arrival;
}

void RadioMedium::removeCachedArrival(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->arrival = NULL;
}

const IListening *RadioMedium::getCachedListening(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->listening : NULL;
}

void RadioMedium::setCachedListening(const IRadio *radio, const ITransmission *transmission, const IListening *listening) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->listening = listening;
}

void RadioMedium::removeCachedListening(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->listening = NULL;
}

const IReception *RadioMedium::getCachedReception(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->reception : NULL;
}

void RadioMedium::setCachedReception(const IRadio *radio, const ITransmission *transmission, const IReception *reception) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->reception = reception;
}

void RadioMedium::removeCachedReception(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->reception = NULL;
}

const IReceptionDecision *RadioMedium::getCachedDecision(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->decision : NULL;
}

void RadioMedium::setCachedDecision(const IRadio *radio, const ITransmission *transmission, const IReceptionDecision *decision) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = decision;
}

void RadioMedium::removeCachedDecision(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = NULL;
}

void RadioMedium::invalidateCachedDecisions(const ITransmission *transmission)
{
    for (std::vector<TransmissionCacheEntry>::iterator it = cache.begin(); it != cache.end(); it++)
    {
        const TransmissionCacheEntry &transmissionCacheEntry = *it;
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        if (receptionCacheEntries)
        {
            for (std::vector<ReceptionCacheEntry>::iterator jt = receptionCacheEntries->begin(); jt != receptionCacheEntries->end(); jt++)
            {
                ReceptionCacheEntry &cacheEntry = *jt;
                const IReceptionDecision *decision = cacheEntry.decision;
                if (decision)
                {
                    const IReception *reception = decision->getReception();
                    if (isInterferingTransmission(transmission, reception))
                        invalidateCachedDecision(decision);
                }
            }
        }
    }
}

void RadioMedium::invalidateCachedDecision(const IReceptionDecision *decision)
{
    const IReception *reception = decision->getReception();
    const IRadio *radio = reception->getReceiver();
    const ITransmission *transmission = reception->getTransmission();
    ReceptionCacheEntry *receptionCacheEntry = getReceptionCacheEntry(radio, transmission);
    if (receptionCacheEntry) receptionCacheEntry->decision = NULL;
}

template<typename T> inline T minIgnoreNaN(T a, T b)
{
    if (isNaN(a.get())) return b;
    else if (isNaN(b.get())) return a;
    else if (a < b) return a;
    else return b;
}

template<typename T> inline T maxIgnoreNaN(T a, T b)
{
    if (isNaN(a.get())) return b;
    else if (isNaN(b.get())) return a;
    else if (a > b) return a;
    else return b;
}

inline double maxIgnoreNaN(double a, double b)
{
    if (isNaN(a)) return b;
    else if (isNaN(b)) return a;
    else if (a > b) return a;
    else return b;
}

W RadioMedium::computeMaxTransmissionPower() const
{
    W maxTransmissionPower = W(par("maxTransmissionPower"));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        maxTransmissionPower = maxIgnoreNaN(maxTransmissionPower, (*it)->getTransmitter()->getMaxPower());
    return maxTransmissionPower;
}

W RadioMedium::computeMinInterferencePower() const
{
    W minInterferencePower = mW(FWMath::dBm2mW(par("minInterferencePower")));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        minInterferencePower = minIgnoreNaN(minInterferencePower, (*it)->getReceiver()->getMinInterferencePower());
    return minInterferencePower;
}

W RadioMedium::computeMinReceptionPower() const
{
    W minReceptionPower = mW(FWMath::dBm2mW(par("minReceptionPower")));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        minReceptionPower = minIgnoreNaN(minReceptionPower, (*it)->getReceiver()->getMinReceptionPower());
    return minReceptionPower;
}

double RadioMedium::computeMaxAntennaGain() const
{
    double maxAntennaGain = FWMath::dB2fraction(par("maxAntennaGain"));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        maxAntennaGain = maxIgnoreNaN(maxAntennaGain, (*it)->getAntenna()->getMaxGain());
    return maxAntennaGain;
}

m RadioMedium::computeMaxRange(W maxTransmissionPower, W minReceptionPower) const
{
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    double loss = unit(minReceptionPower / maxTransmissionPower).get() / maxAntennaGain / maxAntennaGain;
    return pathLoss->computeRange(propagation->getPropagationSpeed(), carrierFrequency, loss);
}

m RadioMedium::computeMaxCommunicationRange() const
{
    return maxIgnoreNaN(m(par("maxCommunicationRange")), computeMaxRange(maxTransmissionPower, minReceptionPower));
}

m RadioMedium::computeMaxInterferenceRange() const
{
    return maxIgnoreNaN(m(par("maxInterferenceRange")), computeMaxRange(maxTransmissionPower, minInterferencePower));
}

const simtime_t RadioMedium::computeMinInterferenceTime() const
{
    return par("minInterferenceTime").doubleValue();
}

const simtime_t RadioMedium::computeMaxTransmissionDuration() const
{
    return par("maxTransmissionDuration").doubleValue();
}

void RadioMedium::updateLimits()
{
    maxTransmissionPower = computeMaxTransmissionPower();
    minInterferencePower = computeMinInterferencePower();
    minReceptionPower = computeMinReceptionPower();
    maxAntennaGain = computeMaxAntennaGain();
    minInterferenceTime = computeMinInterferenceTime();
    maxTransmissionDuration = computeMaxTransmissionDuration();
    maxCommunicationRange = computeMaxCommunicationRange();
    maxInterferenceRange = computeMaxInterferenceRange();
}

bool RadioMedium::isRadioMacAddress(const IRadio *radio, const MACAddress address) const
{
    cModule *host = getContainingNode(const_cast<cModule *>(check_and_cast<const cModule *>(radio)));
    IInterfaceTable *interfaceTable = check_and_cast<IInterfaceTable *>(host->getSubmodule("interfaceTable"));
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
    {
        const InterfaceEntry *interface = interfaceTable->getInterface(i);
        if (interface && interface->getMacAddress() == address)
            return true;
    }
    return false;
}

bool RadioMedium::isInCommunicationRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    return isNaN(maxCommunicationRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxCommunicationRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxCommunicationRange.get());
}

bool RadioMedium::isInInterferenceRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    return isNaN(maxInterferenceRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxInterferenceRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxInterferenceRange.get());
}

bool RadioMedium::isInterferingTransmission(const ITransmission *transmission, const IListening *listening) const
{
    const IRadio *transmitter = transmission->getTransmitter();
    const IRadio *receiver = listening->getReceiver();
    const IArrival *arrival = getArrival(receiver, transmission);
    return transmitter != receiver &&
           arrival->getEndTime() >= listening->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() <= listening->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, listening->getStartPosition(), listening->getEndPosition());
}

bool RadioMedium::isInterferingTransmission(const ITransmission *transmission, const IReception *reception) const
{
    const IRadio *transmitter = transmission->getTransmitter();
    const IRadio *receiver = reception->getReceiver();
    const IArrival *arrival = getArrival(receiver, transmission);
    return transmitter != receiver &&
           arrival->getEndTime() > reception->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() < reception->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, reception->getStartPosition(), reception->getEndPosition());
}

void RadioMedium::removeNonInterferingTransmissions()
{
    const simtime_t now = simTime();
    size_t transmissionIndex = 0;
    while (transmissionIndex < cache.size() && cache[transmissionIndex].interferenceEndTime <= now)
        transmissionIndex++;
    EV_DEBUG << "Removing " << transmissionIndex << " non interfering transmissions\n";
    baseTransmissionId += transmissionIndex;
    transmissions.erase(transmissions.begin(), transmissions.begin() + transmissionIndex);
    for (std::vector<TransmissionCacheEntry>::const_iterator it = cache.begin(); it != cache.begin() + transmissionIndex; it++)
    {
        const TransmissionCacheEntry &transmissionCacheEntry = *it;
        delete transmissionCacheEntry.frame;
        const std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        if (receptionCacheEntries)
        {
            for (std::vector<ReceptionCacheEntry>::const_iterator jt = receptionCacheEntries->begin(); jt != receptionCacheEntries->end(); jt++)
            {
                const ReceptionCacheEntry &cacheEntry = *jt;
                delete cacheEntry.arrival;
                delete cacheEntry.listening;
                delete cacheEntry.reception;
                delete cacheEntry.decision;
            }
            delete receptionCacheEntries;
        }
    }
    cache.erase(cache.begin(), cache.begin() + transmissionIndex);
    if (cache.size() > 0)
        scheduleAt(cache[0].interferenceEndTime, removeNonInterferingTransmissionsTimer);
}

const IReception *RadioMedium::computeReception(const IRadio *radio, const ITransmission *transmission) const
{
    receptionComputationCount++;
    return attenuation->computeReception(radio, transmission);
}

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IListening *listening, const std::vector<const ITransmission *> *transmissions) const
{
    const IRadio *radio = listening->getReceiver();
    std::vector<const IReception *> *interferingReceptions = new std::vector<const IReception *>();
    for (std::vector<const ITransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const ITransmission *interferingTransmission = *it;
        if (interferingTransmission->getTransmitter() != radio && isInterferingTransmission(interferingTransmission, listening))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IReception *reception, const std::vector<const ITransmission *> *transmissions) const
{
    const IRadio *radio = reception->getReceiver();
    const ITransmission *transmission = reception->getTransmission();
    std::vector<const IReception *> *interferingReceptions = new std::vector<const IReception *>();
    for (std::vector<const ITransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const ITransmission *interferingTransmission = *it;
        if (transmission != interferingTransmission && interferingTransmission->getTransmitter() != radio && isInterferingTransmission(interferingTransmission, reception))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const IReceptionDecision *RadioMedium::computeReceptionDecision(const IRadio *radio, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const
{
    receptionDecisionComputationCount++;
    const IReception *reception = getReception(radio, transmission);
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(reception, transmissions);
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : NULL;
    const IReceptionDecision *decision = radio->getReceiver()->computeReceptionDecision(listening, reception, interferingReceptions, noise);
    delete noise;
    delete interferingReceptions;
    return decision;
}

const IListeningDecision *RadioMedium::computeListeningDecision(const IRadio *radio, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const
{
    listeningDecisionComputationCount++;
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(listening, transmissions);
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : NULL;
    const IListeningDecision *decision = radio->getReceiver()->computeListeningDecision(listening, interferingReceptions, noise);
    delete noise;
    delete interferingReceptions;
    return decision;
}

const IReception *RadioMedium::getReception(const IRadio *radio, const ITransmission *transmission) const
{
    cacheReceptionGetCount++;
    const IReception *reception = getCachedReception(radio, transmission);
    if (reception)
        cacheReceptionHitCount++;
    else
    {
        reception = computeReception(radio, transmission);
        setCachedReception(radio, transmission, reception);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << reception << endl;
    }
    return reception;
}

const IReceptionDecision *RadioMedium::getReceptionDecision(const IRadio *radio, const IListening *listening, const ITransmission *transmission) const
{
    cacheDecisionGetCount++;
    const IReceptionDecision *decision = getCachedDecision(radio, transmission);
    if (decision)
        cacheDecisionHitCount++;
    else
    {
        decision = computeReceptionDecision(radio, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
        setCachedDecision(radio, transmission, decision);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << decision->getReception() << " and results in " << decision << endl;
    }
    return decision;
}

void RadioMedium::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
    {
        const ITransmission *transmission = *it;
        const IArrival *arrival = propagation->computeArrival(transmission, radio->getAntenna()->getMobility());
        const IListening *listening = radio->getReceiver()->createListening(radio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
        setCachedArrival(radio, transmission, arrival);
        setCachedListening(radio, transmission, listening);
    }
    if (initialized())
        updateLimits();
    cModule *radioModule = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
    if (radioModeFilter)
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
    if (listeningFilter)
        radioModule->subscribe(IRadio::listeningChangedSignal, this);
    if (macAddressFilter)
        getContainingNode(radioModule)->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
    if (neighborCache)
        neighborCache->addRadio(radio);
}

void RadioMedium::removeRadio(const IRadio *radio)
{
    radios.erase(std::remove(radios.begin(), radios.end(), radio));
    if (initialized())
        updateLimits();
    if (neighborCache)
        neighborCache->removeRadio(radio);
}

void RadioMedium::addTransmission(const IRadio *transmitterRadio, const ITransmission *transmission)
{
    transmissionCount++;
    transmissions.push_back(transmission);
    simtime_t maxArrivalEndTime = simTime();
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IRadio *receiverRadio = *it;
        if (receiverRadio != transmitterRadio)
        {
            const IArrival *arrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
            const simtime_t arrivalEndTime = arrival->getEndTime();
            if (arrivalEndTime > maxArrivalEndTime)
                maxArrivalEndTime = arrivalEndTime;
            setCachedArrival(receiverRadio, transmission, arrival);
            setCachedListening(receiverRadio, transmission, listening);
        }
    }
    getTransmissionCacheEntry(transmission)->interferenceEndTime = maxArrivalEndTime + maxTransmissionDuration;
    if (!removeNonInterferingTransmissionsTimer->isScheduled())
    {
        Enter_Method_Silent();
        scheduleAt(cache[0].interferenceEndTime, removeNonInterferingTransmissionsTimer);
    }
}

void RadioMedium::sendToAffectedRadios(IRadio *radio, const IRadioFrame *frame)
{
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(frame);
    EV_DEBUG << "Sending " << frame << " with " << radioFrame->getBitLength() << " bits in " << radioFrame->getDuration() * 1E+6 << " us transmission duration"
             << " from " << radio << " on " << (IRadioMedium *)this << "." << endl;
    if (neighborCache)
        neighborCache->sendToNeighbors(radio, frame);
    else
        for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
            sendToRadio(radio, *it, frame);
}

void RadioMedium::sendToRadio(IRadio *transmitter, const IRadio *receiver, const IRadioFrame *frame)
{
    const Radio *transmitterRadio = check_and_cast<const Radio *>(transmitter);
    const Radio *receiverRadio = check_and_cast<const Radio *>(receiver);
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(frame);
    const ITransmission *transmission = frame->getTransmission();
    ReceptionCacheEntry *receptionCacheEntry = getReceptionCacheEntry(receiverRadio, transmission);
    if (receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
    {
        const IArrival *arrival = getArrival(receiverRadio, transmission);
        simtime_t propagationTime = arrival->getStartPropagationTime();
        EV_DEBUG << "Sending " << frame
                 << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                 << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                 << " in " << propagationTime * 1E+6 << " us propagation time." << endl;
        RadioFrame *frameCopy = radioFrame->dup();
        cGate *gate = receiverRadio->getRadioGate()->getPathStartGate();
        const_cast<Radio *>(transmitterRadio)->sendDirect(frameCopy, propagationTime, radioFrame->getDuration(), gate);
        receptionCacheEntry->frame = frameCopy;
        sendCount++;
    }
}

IRadioFrame *RadioMedium::transmitPacket(const IRadio *radio, cPacket *macFrame)
{
    const ITransmission *transmission = radio->getTransmitter()->createTransmission(radio, macFrame, simTime());
    addTransmission(radio, transmission);
    RadioFrame *radioFrame = new RadioFrame(transmission);
    radioFrame->setName(macFrame->getName());
    radioFrame->setDuration(transmission->getEndTime() - transmission->getStartTime());
    radioFrame->encapsulate(macFrame);
    if (recordCommunication) {
        const Radio *transmitterRadio = check_and_cast<const Radio *>(radio);
        communicationLog << "T " << transmitterRadio->getFullPath() << " " << transmitterRadio->getId() << " "
                         << "M " << radioFrame->getName() << " " << transmission->getId() << " "
                         << "S " << transmission->getStartTime() << " " <<  transmission->getStartPosition() << " -> "
                         << "E " << transmission->getEndTime() << " " <<  transmission->getEndPosition() << endl;
    }
    sendToAffectedRadios(const_cast<IRadio *>(radio), radioFrame);
    cMethodCallContextSwitcher contextSwitcher(this);
    getTransmissionCacheEntry(transmission)->frame = radioFrame->dup();
    return radioFrame;
}

cPacket *RadioMedium::receivePacket(const IRadio *radio, IRadioFrame *radioFrame)
{
    const ITransmission *transmission = radioFrame->getTransmission();
    const IArrival *arrival = getArrival(radio, transmission);
    const IListening *listening = radio->getReceiver()->createListening(radio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
    const IReceptionDecision *decision = receiveFromMedium(radio, listening, transmission);
    if (recordCommunication) {
        const IReception *reception = decision->getReception();
        communicationLog << "R " << check_and_cast<const Radio *>(radio)->getFullPath() << " " << reception->getReceiver()->getId() << " "
                         << "M " << check_and_cast<const RadioFrame *>(radioFrame)->getName() << " " << transmission->getId() << " "
                         << "S " << reception->getStartTime() << " " <<  reception->getStartPosition() << " -> "
                         << "E " << reception->getEndTime() << " " <<  reception->getEndPosition() << " "
                         << "D " << decision->isReceptionPossible() << " " << decision->isReceptionAttempted() << " " << decision->isReceptionSuccessful() << endl;
    }
    cPacket *macFrame = check_and_cast<cPacket *>(radioFrame)->decapsulate();
    macFrame->setBitError(!decision->isReceptionSuccessful());
    macFrame->setControlInfo(const_cast<RadioReceptionIndication *>(decision->getIndication()));
    delete listening;
    return macFrame;
}

const IReceptionDecision *RadioMedium::receiveFromMedium(const IRadio *radio, const IListening *listening, const ITransmission *transmission) const
{
    const IReceptionDecision *decision = getReceptionDecision(radio, listening, transmission);
    removeCachedDecision(radio, transmission);
    return decision;
}

const IListeningDecision *RadioMedium::listenOnMedium(const IRadio *radio, const IListening *listening) const
{
    const IListeningDecision *decision = computeListeningDecision(radio, listening, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    EV_DEBUG << "Listening " << listening << " on medium by " << radio << " results in " << decision << endl;
    return decision;
}

bool RadioMedium::isPotentialReceiver(const IRadio *radio, const ITransmission *transmission) const
{
    const Radio *receiverRadio = check_and_cast<const Radio *>(radio);
    if (radioModeFilter && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_TRANSCEIVER)
        return false;
    else if (listeningFilter && !radio->getReceiver()->computeIsReceptionPossible(transmission))
        return false;
    else if (macAddressFilter && !isRadioMacAddress(radio, check_and_cast<const IMACFrame *>(transmission->getMacFrame())->getDestinationAddress()))
        return false;
    else if (rangeFilter == RANGE_FILTER_INTERFERENCE_RANGE) {
        const IArrival *arrival = getArrival(radio, transmission);
        return isInInterferenceRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
    }
    else if (rangeFilter == RANGE_FILTER_COMMUNICATION_RANGE) {
        const IArrival *arrival = getArrival(radio, transmission);
        return isInCommunicationRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
    }
    else
        return true;
}

bool RadioMedium::isReceptionAttempted(const IRadio *radio, const ITransmission *transmission) const
{
    const IReception *reception = getReception(radio, transmission);
    const IListening *listening = getCachedListening(radio, transmission);
    // TODO: isn't there a better way for this optimization? see also in ReceiverBase::computeIsReceptionAttempted
    const std::vector<const IReception *> *interferingReceptions = simTime() == reception->getStartTime() ? NULL : computeInterferingReceptions(reception, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    bool isReceptionAttempted = radio->getReceiver()->computeIsReceptionAttempted(listening, reception, interferingReceptions);
    delete interferingReceptions;
    EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << reception << " and results in reception is " << (isReceptionAttempted ? "attempted" : "ignored") << endl;
    return isReceptionAttempted;
}

const IArrival *RadioMedium::getArrival(const IRadio *radio, const ITransmission *transmission) const
{
    return getCachedArrival(radio, transmission);
}

void RadioMedium::receiveSignal(cComponent *source, simsignal_t signal, long value)
{
    if (signal == IRadio::radioModeChangedSignal || signal == IRadio::listeningChangedSignal || signal == NF_INTERFACE_CONFIG_CHANGED)
    {
        const Radio *receiverRadio = check_and_cast<const Radio *>(source);
        for (std::vector<const ITransmission *>::iterator it = transmissions.begin(); it != transmissions.end(); it++)
        {
            const ITransmission *transmission = *it;
            const Radio *transmitterRadio = check_and_cast<const Radio *>(transmission->getTransmitter());
            ReceptionCacheEntry *receptionCacheEntry = getReceptionCacheEntry(receiverRadio, transmission);
            if (receptionCacheEntry && signal == IRadio::listeningChangedSignal)
            {
                const IArrival *arrival = getArrival(receiverRadio, transmission);
                const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
                delete getCachedListening(receiverRadio, transmission);
                setCachedListening(receiverRadio, transmission, listening);
            }
            if (receptionCacheEntry && !receptionCacheEntry->frame && receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
            {
                const IArrival *arrival = getArrival(receiverRadio, transmission);
                if (arrival->getEndTime() >= simTime())
                {
                    cMethodCallContextSwitcher contextSwitcher(transmitterRadio);
                    contextSwitcher.methodCallSilent();
                    const cPacket *macFrame = transmission->getMacFrame();
                    EV_DEBUG << "Picking up " << macFrame << " originally sent "
                             << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                             << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                             << " in " << arrival->getStartPropagationTime() * 1E+6 << " us propagation time." << endl;
                    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
                    RadioFrame *frameCopy = dynamic_cast<const RadioFrame *>(transmissionCacheEntry->frame)->dup();
                    simtime_t delay = arrival->getStartTime() - simTime();
                    simtime_t duration = delay > 0 ? frameCopy->getDuration() : frameCopy->getDuration() + delay;
                    cGate *gate = receiverRadio->getRadioGate()->getPathStartGate();
                    const_cast<Radio *>(transmitterRadio)->sendDirect(frameCopy, delay > 0 ? delay : 0, duration, gate);
                    receptionCacheEntry->frame = frameCopy;
                    sendCount++;
                }
            }
        }
    }
}


} // namespace inet

