#include "TChannelInfo.hxx"

#include <TCaptLog.hxx>
#include <CaptGeomId.hxx>
#include <TChannelId.hxx>
#include <TMCChannelId.hxx>
#include <TTPCChannelId.hxx>
#include <TGeometryId.hxx>

#include <TSystem.h>

#include <fstream>
#include <string>
#include <sstream>

// Initialize the singleton pointer.
CP::TChannelInfo* CP::TChannelInfo::fChannelInfo = NULL;

CP::TChannelInfo& CP::TChannelInfo::Get() {
    if (!fChannelInfo) {
        CaptVerbose("Create a new CP::TChannelInfo object");
        fChannelInfo = new CP::TChannelInfo();
    }
    return *fChannelInfo;
}

CP::TChannelInfo::TChannelInfo() {
    const char* envVal = gSystem->Getenv("CAPTCHANNELMAP");
    if (!envVal) return;

    std::string mapName(envVal);

    if (mapName.empty()) return;

    // Attache the file to a stream.
    std::ifstream mapFile(mapName.c_str());
    std::string line;
    while (std::getline(mapFile,line)) {
        std::size_t comment = line.find("#");
        std::string tmp = line;
        if (comment != std::string::npos) tmp.erase(comment);
        // The minimum valid date, time and hash is 22 characters.  
        if (tmp.size()<20) continue;
        std::istringstream parser(tmp);
        int crate;
        int fem;
        int channel;
        int detector;
        int plane;
        int wire;
        
        parser >> crate;
        if (parser.fail()) {
            CaptError("Could not parse crate number: "<< line);
            continue;
        }

        parser >> fem;
        if (parser.fail()) {
            CaptError("Could not parse fem number: "<< line);
            continue;
        }

        parser >> channel;
        if (parser.fail()) {
            CaptError("Could not parse channel number: "<< line);
            continue;
        }

        parser >> detector;
        if (parser.fail()) {
            CaptError("Could not parse detector number: "<< line);
            continue;
        }

        parser >> plane;
        if (parser.fail()) {
            CaptError("Could not parse plane number: "<< line);
            continue;
        }

        parser >> wire;
        if (parser.fail()) {
            CaptError("Could not parse wire number: "<< line);
            continue;
        }

        CP::TChannelId cid;
        CP::TGeometryId gid;
        if (detector == CP::TChannelId::kTPC) {
            cid = CP::TTPCChannelId(crate,fem,channel);
            gid = CP::GeomId::Captain::Wire(plane,wire);
        }
        else {
            CaptError("Unknown detector channel: " << line);
            continue;
        }

        if (fChannelMap.find(cid) != fChannelMap.end()) {
            CaptError("Channel already exists: " << line);
            CaptError("   Duplicate " << gid);
        }

        if (fGeometryMap.find(gid) != fGeometryMap.end()) {
            CaptError("Channel already exists: " << line);
            CaptError("   Duplicate " << cid);
        }

        fChannelMap[cid] = gid;
        fGeometryMap[gid] = cid;

    }
}

void CP::TChannelInfo::SetContext(const CP::TEventContext& context) {
    // This needs to check if a new mapping needs to be loaded.
    fContext = context;
}

const CP::TEventContext& CP::TChannelInfo::GetContext() const {
    if (fContext.IsValid()) return fContext;
    CaptError("Event context must be set before using TChannelInfo.");
    return fContext;
}

CP::TChannelId CP::TChannelInfo::GetChannel(CP::TGeometryId gid, int index) {
    // At the moment, index is always zero.
    if (index != 0) return CP::TChannelId();

    // Make sure that the identifier is valid.
    if (!gid.IsValid()) {
        CaptError("Invalid geometry id cannot be translated to a channel");
        return CP::TChannelId();
    }

    // Make sure that we know what the current context is.  The channel to
    // geometry mapping changes with time.
    if (!GetContext().IsValid()) {
        CaptError("Need valid event context to translate geometry to channel");
        return CP::TChannelId();
    }

    // The current context is for the MC, so the channel can be generated
    // algorithmically.
    if (GetContext().IsMC()) {
        if (CP::GeomId::Captain::IsWire(gid)) {
            return CP::TMCChannelId(
                0,
                CP::GeomId::Captain::GetWirePlane(gid),
                CP::GeomId::Captain::GetWireNumber(gid));
        }
        else if (CP::GeomId::Captain::IsPhotosensor(gid)) {
            return CP::TMCChannelId(
                1, 0,
                CP::GeomId::Captain::GetPhotosensor(gid));
        }
        return  CP::TChannelId();
    }
    
    // The context is valid, and not for the MC, so it should be for the
    // detector.  This shouldn't never happen, but it might.
#ifdef CHECK_DETECTOR_CONTEXT
    if (!GetContext().IsDetector()) {
        CaptError("Channel requested for invalid event context");
        return CP::TChannelId();
    }
#endif

    std::map<CP::TGeometryId,CP::TChannelId>::iterator geometryEntry
        = fGeometryMap.find(gid);

    if (geometryEntry == fGeometryMap.end()) {
        CaptError("Channel for object not found: " << gid);
        return CP::TChannelId();
    }
        
    return geometryEntry->second;
}

int CP::TChannelInfo::GetChannelCount(CP::TGeometryId id) {
    if (!id.IsValid()) return 0;
    return 1;
}

CP::TGeometryId CP::TChannelInfo::GetGeometry(CP::TChannelId cid, int index) {
    // At the moment, index is always zero.
    if (index != 0) return CP::TGeometryId();

    // Make sure that we have an event context since the channel to geometry
    // mapping changes with time.
    if (!GetContext().IsValid()) {
        CaptWarn("Need valid event context to translate channel to geometry");
        return CP::TGeometryId();
    }

    // Make sure this is a valid channel and flag an error if not.
    if (!cid.IsValid()) {
        CaptError("Invalid channel cannot be translated to geometry");
        return CP::TGeometryId();
    }

    // The current context is for the MC, so the channel can be generated
    // algorithmically.
    if (cid.IsMCChannel()) {
        CP::TMCChannelId id(cid);
        if (id.GetType() == 0) {
            // This is a wire channel.
            return CP::GeomId::Captain::Wire(id.GetSequence(),id.GetNumber());
        }
        else if (id.GetType() == 1) {
            // This is a light sensor.
            return CP::GeomId::Captain::Photosensor(id.GetNumber());
        }
        return  CP::TGeometryId();
    }
    
    // The context is valid, and not for the MC, so it should be for the
    // detector.  This shouldn't never happen, but it might.
#ifdef CHECK_DETECTOR_CONTEXT
    if (!GetContext().IsDetector()) {
        CaptError("Channel requested for invalid event context");
        return CP::TGeometryId();
    }
#endif

    std::map<CP::TChannelId, CP::TGeometryId>::iterator channelEntry
        = fChannelMap.find(cid);

    if (channelEntry == fChannelMap.end()) {
        CaptError("Geometry for channel is not found: " << cid);
        return CP::TGeometryId();
    }
        
    return channelEntry->second;
}

int CP::TChannelInfo::GetGeometryCount(CP::TChannelId id) {
    if (!id.IsValid()) return 0;
    return 1;
}
