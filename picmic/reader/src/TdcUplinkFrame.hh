#pragma once

#include <array>
#include <cstdint>
#include <string>

/**
 * Représentation d'une trame TDC uplink de 256 bits.
 *
 * Organisation mémoire :
 *
 * word[0] -> bits   31..0
 * word[1] -> bits   63..32
 * ...
 * word[7] -> bits 255..224
 *
 * Correspond exactement au code Python :
 *
 * frame_256 =
 *      (frame_32[7]<<224) |
 *      ...
 *      frame_32[0]
 *
 */
class TdcUplinkFrame
{
public:
    using FrameWords = std::array<uint32_t,8>;

    explicit TdcUplinkFrame(const FrameWords& words);

    /// Retourne un bit unique
    bool bit(unsigned pos) const;

    /// Retourne un champ [msb:lsb] inclus
    uint32_t get(unsigned msb, unsigned lsb) const;

    /// Debug
    std::string toString() const;

    // ============================================================
    // Header
    // ============================================================

    uint32_t bc0id() const;

    // ============================================================
    // FEB1
    // ============================================================

    bool feb1_resync_lb() const;
    bool feb1_bc0_lb() const;

    uint32_t feb1_overflow() const;

    bool feb1_scframe() const;

    bool feb1_strip_0() const;
    bool feb1_strip_1() const;

    bool feb1_dvalid_0() const;
    bool feb1_dvalid_1() const;
    bool feb1_dvalid_2() const;

    uint32_t feb1_devaddr_0() const;
    uint32_t feb1_devaddr_1() const;
    uint32_t feb1_devaddr_2() const;

    uint32_t feb1_chanid_0() const;
    uint32_t feb1_chanid_1() const;
    uint32_t feb1_chanid_2() const;

    uint32_t feb1_tdc_data_0() const;
    uint32_t feb1_tdc_data_1() const;
    uint32_t feb1_tdc_data_2() const;

    uint32_t feb1_diff_0() const;
    uint32_t feb1_diff_1() const;

    // ============================================================
    // FEB0
    // ============================================================

    bool feb0_resync_lb() const;
    bool feb0_bc0_lb() const;

    uint32_t feb0_overflow() const;

    bool feb0_scframe() const;

    bool feb0_strip_0() const;
    bool feb0_strip_1() const;

    bool feb0_dvalid_0() const;
    bool feb0_dvalid_1() const;
    bool feb0_dvalid_2() const;

    uint32_t feb0_devaddr_0() const;
    uint32_t feb0_devaddr_1() const;
    uint32_t feb0_devaddr_2() const;

    uint32_t feb0_chanid_0() const;
    uint32_t feb0_chanid_1() const;
    uint32_t feb0_chanid_2() const;

    uint32_t feb0_tdc_data_0() const;
    uint32_t feb0_tdc_data_1() const;
    uint32_t feb0_tdc_data_2() const;

    uint32_t feb0_diff_0() const;
    uint32_t feb0_diff_1() const;

private:
    FrameWords words_;
};
