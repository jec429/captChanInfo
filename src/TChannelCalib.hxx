#ifndef TChannelCalib_hxx_seen
#define TChannelCalib_hxx_seen

#include <ECore.hxx>

namespace CP {
    class TChannelCalib;
    class TChannelId;
};

namespace CP {
    EXCEPTION(EChannelCalib,ECore);
    EXCEPTION(EChannelCalibUnknownType, EChannelCalib);
};

/// Provide generic an interface to get the calibration coefficients.  This
/// works for calibration constants provided in an MC file, and for constants
/// for the data.
class CP::TChannelCalib {
public:
    TChannelCalib();
    ~TChannelCalib();

    /// This is true if the channel is considered good.
    bool IsGoodChannel(CP::TChannelId id);

    /// This returns true if the signal is a bipolar signal.  The collection
    /// wires and PMTs are unipolar.  The induction wires are bipolar.
    bool IsBipolarSignal(CP::TChannelId id);

    /// The a summary of the status flags for the channel.  This combines the
    /// information derived during calibration with the hand modified
    /// information in the TPC_BAD_CHANNEL_TABLE
    int GetChannelStatus(CP::TChannelId id);
    
    /// Get the amplifier gain constants for a channel.  The second parameter
    /// is the order of the constant.  Normally, order 0 is the offset for the
    /// charge calibration (always zero), order 1 is linear, order 2 is
    /// quadratic, etc, however, the definitions depend on the calibration
    /// model.  For instance, in the future, we might use model with a
    /// logarithmic saturation.
    ///
    /// In the TPC electronics, the linear term has units of
    /// (voltage)/(charge).
    double GetGainConstant(CP::TChannelId id, int order=1);

    /// Get the time constants for a channel.  These are the calibration
    /// constants for the time per digitizer sample.  The nominal value is 500
    /// ns. The second parameter is the order of the constant.  Normally,
    /// order 0 is the pedestal, order 1 is linear, order 2 is quadratic, etc.
    double GetTimeConstant(CP::TChannelId id, int order=1);

    /// Get the digitizer constant to convert ADC to voltage.  The second
    /// parameter is the order of the constant.  Normally, order 0 is the
    /// digitizer pedestal (usually ~2048 or ~450), order 1 is linear
    /// (ADC/volt), order 2 is quadratic, etc.
    double GetDigitizerConstant(CP::TChannelId id, int order=1);

    /// Get the electron lifetime.
    double GetElectronLifetime();

    /// Get the electron drift velocity.  This can be calculated by disabling
    /// the drift calibration for CLUSTERCALIB.exe using the -O no-drift flag.
    /// The average X charge vs time can then be fit to determine the drift
    /// lifetime.  
    double GetElectronDriftVelocity();

    /// Get the wire collection efficiency.  This can be calculated by
    /// disabling the efficiency calibration for CLUSTERCALIB.exe using the -O
    /// no-efficiency flag.  The collection efficiency is then the ratio
    /// between the U/X and V/X ratios.  This can also include wire-to-wire
    /// differences, but those are not calculated with CLUSTERCALIB.exe.
    double GetCollectionEfficiency(CP::TChannelId id);

    /// Get the pulse shaping for the ASIC as a function of time.
    /// microsecond.
    double GetPulseShape(CP::TChannelId id, double t);

    /// Get the peaking time for the ASIC.  This shouldn't be used directly.
    /// Access the pulse shape through GetPulseShape.
    double GetPulseShapePeakTime(CP::TChannelId id, int order = 0);

    /// Get the shape factor for the rising edge of the ASIC shaping.  This
    /// shouldn't be used directly.  Access the pulse shape through
    /// GetPulseShape.
    double GetPulseShapeRise(CP::TChannelId id, int order = 0);
    
    /// Get the shape factor for the fallng edge of the ASIC shaping.  This
    /// shouldn't be used directly.  Access the pulse shape through
    /// GetPulseShape.
    double GetPulseShapeFall(CP::TChannelId id, int order = 0);

};

#endif
