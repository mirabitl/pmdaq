#include "TdcUplinkFrame.hh"

#include <sstream>
#include <iomanip>
#include <stdexcept>

TdcUplinkFrame::TdcUplinkFrame(const FrameWords& words)
    : words_(words)
{
}

bool TdcUplinkFrame::bit(unsigned pos) const
{
    unsigned word = pos / 32;
    unsigned b    = pos % 32;

    if (word >= 8)
        throw std::out_of_range("TdcUplinkFrame::bit");

    return (words_[word] >> b) & 0x1;
}

uint32_t TdcUplinkFrame::get(unsigned msb, unsigned lsb) const
{
    if (msb < lsb)
        std::swap(msb, lsb);

    uint32_t value = 0;
    unsigned outBit = 0;

    for (unsigned b = lsb; b <= msb; ++b)
    {
        unsigned word = b / 32;
        unsigned bitpos = b % 32;

        uint32_t bitval = (words_[word] >> bitpos) & 0x1;

        value |= (bitval << outBit);

        ++outBit;

        if (outBit >= 32)
            break;
    }

    return value;
}

std::string TdcUplinkFrame::toString() const
{
    std::ostringstream os;

    for (int i = 7; i >= 0; --i)
    {
        os << std::hex
           << std::setw(8)
           << std::setfill('0')
           << words_[i];
    }

    return os.str();
}

//
// ============================================================
// Header
// ============================================================
//

uint32_t TdcUplinkFrame::bc0id() const
{
    return get(255,224);
}

//
// ============================================================
// FEB1
// ============================================================
//

bool TdcUplinkFrame::feb1_resync_lb() const
{
    return bit(223);
}

bool TdcUplinkFrame::feb1_bc0_lb() const
{
    return bit(222);
}

uint32_t TdcUplinkFrame::feb1_overflow() const
{
    return get(221,218);
}

bool TdcUplinkFrame::feb1_scframe() const
{
    return bit(214);
}

bool TdcUplinkFrame::feb1_strip_0() const
{
    return bit(213);
}

bool TdcUplinkFrame::feb1_strip_1() const
{
    return bit(212);
}

bool TdcUplinkFrame::feb1_dvalid_0() const
{
    return bit(210);
}

bool TdcUplinkFrame::feb1_dvalid_1() const
{
    return bit(209);
}

bool TdcUplinkFrame::feb1_dvalid_2() const
{
    return bit(208);
}

uint32_t TdcUplinkFrame::feb1_devaddr_0() const
{
    return get(207,206);
}

uint32_t TdcUplinkFrame::feb1_chanid_0() const
{
    return get(205,200);
}

uint32_t TdcUplinkFrame::feb1_tdc_data_0() const
{
    return get(199,176);
}

uint32_t TdcUplinkFrame::feb1_devaddr_1() const
{
    return get(175,174);
}

uint32_t TdcUplinkFrame::feb1_chanid_1() const
{
    return get(173,168);
}

uint32_t TdcUplinkFrame::feb1_tdc_data_1() const
{
    return get(167,144);
}

uint32_t TdcUplinkFrame::feb1_devaddr_2() const
{
    return get(143,142);
}

uint32_t TdcUplinkFrame::feb1_chanid_2() const
{
    return get(141,136);
}

uint32_t TdcUplinkFrame::feb1_tdc_data_2() const
{
    return get(135,112);
}

uint32_t TdcUplinkFrame::feb1_diff_0() const
{
    return get(143,128);
}

uint32_t TdcUplinkFrame::feb1_diff_1() const
{
    return get(127,112);
}

//
// ============================================================
// FEB0
// ============================================================
//

bool TdcUplinkFrame::feb0_resync_lb() const
{
    return bit(111);
}

bool TdcUplinkFrame::feb0_bc0_lb() const
{
    return bit(110);
}

uint32_t TdcUplinkFrame::feb0_overflow() const
{
    //
    // Le python contient visiblement un copier/coller :
    //
    // "feb1_overflow" : (221,218)
    //
    // dans la zone FEB0.
    //
    // Je garde ici la version symétrique attendue.
    //
    return get(109,106);
}

bool TdcUplinkFrame::feb0_scframe() const
{
    return bit(102);
}

bool TdcUplinkFrame::feb0_strip_0() const
{
    return bit(101);
}

bool TdcUplinkFrame::feb0_strip_1() const
{
    return bit(100);
}

bool TdcUplinkFrame::feb0_dvalid_0() const
{
    return bit(98);
}

bool TdcUplinkFrame::feb0_dvalid_1() const
{
    return bit(97);
}

bool TdcUplinkFrame::feb0_dvalid_2() const
{
    return bit(96);
}

uint32_t TdcUplinkFrame::feb0_devaddr_0() const
{
    return get(95,94);
}

uint32_t TdcUplinkFrame::feb0_chanid_0() const
{
    return get(93,88);
}

uint32_t TdcUplinkFrame::feb0_tdc_data_0() const
{
    return get(87,64);
}

uint32_t TdcUplinkFrame::feb0_devaddr_1() const
{
    return get(63,62);
}

uint32_t TdcUplinkFrame::feb0_chanid_1() const
{
    return get(61,56);
}

uint32_t TdcUplinkFrame::feb0_tdc_data_1() const
{
    return get(55,32);
}

uint32_t TdcUplinkFrame::feb0_devaddr_2() const
{
    return get(31,30);
}

uint32_t TdcUplinkFrame::feb0_chanid_2() const
{
    return get(29,24);
}

uint32_t TdcUplinkFrame::feb0_tdc_data_2() const
{
    return get(23,0);
}

uint32_t TdcUplinkFrame::feb0_diff_0() const
{
    return get(31,16);
}

uint32_t TdcUplinkFrame::feb0_diff_1() const
{
    return get(15,0);
}
