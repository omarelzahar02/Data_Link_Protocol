//
// Generated file, do not edit! Created by opp_msgtool 6.0 from initMsg.msg.
//

#ifndef __INITMSG_M_H
#define __INITMSG_M_H

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION != OMNETPP_VERSION)
#error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class InitMsg;
// cplusplus {{
#include "./protocol.h"
// }}

/**
 * Class generated from <tt>initMsg.msg:23</tt> by opp_msgtool.
 * <pre>
 * packet InitMsg
 * {
 *     \@customize(true);  // see the generated C++ header for more info
 *     int startTime;
 *     int type;
 * }
 * </pre>
 *
 * InitMsg_Base is only useful if it gets subclassed, and InitMsg is derived from it.
 * The minimum code to be written for InitMsg is the following:
 *
 * <pre>
 * class InitMsg : public InitMsg_Base
 * {
 *   private:
 *     void copy(const InitMsg& other) { ... }

 *   public:
 *     InitMsg(const char *name=nullptr, short kind=0) : InitMsg_Base(name,kind) {}
 *     InitMsg(const InitMsg& other) : InitMsg_Base(other) {copy(other);}
 *     InitMsg& operator=(const InitMsg& other) {if (this==&other) return *this; InitMsg_Base::operator=(other); copy(other); return *this;}
 *     virtual InitMsg *dup() const override {return new InitMsg(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from InitMsg_Base
 * };
 * </pre>
 *
 * The following should go into a .cc (.cpp) file:
 *
 * <pre>
 * Register_Class(InitMsg)
 * </pre>
 */
class InitMsg_Base : public ::omnetpp::cPacket {
   protected:
    timing startTime = 0;
    node_type type = SENDER;

   private:
    void copy(const InitMsg_Base &other);

   protected:
    bool operator==(const InitMsg_Base &) = delete;
    // make constructors protected to avoid instantiation
    // make assignment operator protected to force the user override it
    InitMsg_Base &operator=(const InitMsg_Base &other);

   public:
    InitMsg_Base(const char *name = nullptr, short kind = 0);
    InitMsg_Base(const InitMsg_Base &other);
    virtual ~InitMsg_Base();
    virtual InitMsg_Base *dup() const override { return new InitMsg_Base(*this); }
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual timing getStartTime() const;
    virtual void setStartTime(timing startTime);

    virtual node_type getType() const;
    virtual void setType(node_type type);
};

namespace omnetpp {

template <>
inline InitMsg_Base *fromAnyPtr(any_ptr ptr) { return check_and_cast<InitMsg_Base *>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif  // ifndef __INITMSG_M_H
