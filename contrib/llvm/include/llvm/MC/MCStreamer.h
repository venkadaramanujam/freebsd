//===- MCStreamer.h - High-level Streaming Machine Code Output --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCSTREAMER_H
#define LLVM_MC_MCSTREAMER_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
  class MCContext;
  class MCValue;
  class MCInst;
  class MCSection;
  class MCSymbol;
  class raw_ostream;

  /// MCStreamer - Streaming machine code generation interface.
  class MCStreamer {
  public:
    enum SymbolAttr {
      Global,         /// .globl
      Hidden,         /// .hidden (ELF)
      IndirectSymbol, /// .indirect_symbol (Apple)
      Internal,       /// .internal (ELF)
      LazyReference,  /// .lazy_reference (Apple)
      NoDeadStrip,    /// .no_dead_strip (Apple)
      PrivateExtern,  /// .private_extern (Apple)
      Protected,      /// .protected (ELF)
      Reference,      /// .reference (Apple)
      Weak,           /// .weak
      WeakDefinition, /// .weak_definition (Apple)
      WeakReference,  /// .weak_reference (Apple)

      SymbolAttrFirst = Global,
      SymbolAttrLast = WeakReference
    };

  private:
    MCContext &Context;

    MCStreamer(const MCStreamer&); // DO NOT IMPLEMENT
    MCStreamer &operator=(const MCStreamer&); // DO NOT IMPLEMENT

  protected:
    MCStreamer(MCContext &Ctx);

  public:
    virtual ~MCStreamer();

    MCContext &getContext() const { return Context; }

    /// @name Symbol & Section Management
    /// @{

    /// SwitchSection - Set the current section where code is being emitted to
    /// @param Section.
    ///
    /// This corresponds to assembler directives like .section, .text, etc.
    virtual void SwitchSection(MCSection *Section) = 0;

    /// EmitLabel - Emit a label for @param Symbol into the current section.
    ///
    /// This corresponds to an assembler statement such as:
    ///   foo:
    ///
    /// @param Symbol - The symbol to emit. A given symbol should only be
    /// emitted as a label once, and symbols emitted as a label should never be
    /// used in an assignment.
    //
    // FIXME: What to do about the current section? Should we get rid of the
    // symbol section in the constructor and initialize it here?
    virtual void EmitLabel(MCSymbol *Symbol) = 0;

    /// EmitAssignment - Emit an assignment of @param Value to @param Symbol.
    ///
    /// This corresponds to an assembler statement such as:
    ///  symbol = value
    ///
    /// The assignment generates no code, but has the side effect of binding the
    /// value in the current context. For the assembly streamer, this prints the
    /// binding into the .s file.
    ///
    /// @param Symbol - The symbol being assigned to.
    /// @param Value - The value for the symbol.
    /// @param MakeAbsolute - If true, then the symbol should be given the
    /// absolute value of @param Value, even if @param Value would be
    /// relocatable expression. This corresponds to the ".set" directive.
    virtual void EmitAssignment(MCSymbol *Symbol, const MCValue &Value,
                                bool MakeAbsolute = false) = 0;

    /// EmitSymbolAttribute - Add the given @param Attribute to @param Symbol.
    //
    // FIXME: This doesn't make much sense, could we just have attributes be on
    // the symbol and make the printer smart enough to add the right symbols?
    // This should work as long as the order of attributes in the file doesn't
    // matter.
    virtual void EmitSymbolAttribute(MCSymbol *Symbol,
                                     SymbolAttr Attribute) = 0;

    /// @}
    /// @name Generating Data
    /// @{

    /// EmitBytes - Emit @param Length bytes starting at @param Data into the
    /// output.
    ///
    /// This is used to implement assembler directives such as .byte, .ascii,
    /// etc.
    virtual void EmitBytes(const char *Data, unsigned Length) = 0;

    /// EmitValue - Emit the expression @param Value into the output as a native
    /// integer of the given @param Size bytes.
    ///
    /// This is used to implement assembler directives such as .word, .quad,
    /// etc.
    ///
    /// @param Value - The value to emit.
    /// @param Size - The size of the integer (in bytes) to emit. This must
    /// match a native machine width.
    virtual void EmitValue(const MCValue &Value, unsigned Size) = 0;

    /// EmitValueToAlignment - Emit some number of copies of @param Value until
    /// the byte alignment @param ByteAlignment is reached.
    ///
    /// If the number of bytes need to emit for the alignment is not a multiple
    /// of @param ValueSize, then the contents of the emitted fill bytes is
    /// undefined.
    ///
    /// This used to implement the .align assembler directive.
    ///
    /// @param ByteAlignment - The alignment to reach. This must be a power of
    /// two.
    /// @param Value - The value to use when filling bytes.
    /// @param Size - The size of the integer (in bytes) to emit for @param
    /// Value. This must match a native machine width.
    /// @param MaxBytesToEmit - The maximum numbers of bytes to emit, or 0. If
    /// the alignment cannot be reached in this many bytes, no bytes are
    /// emitted.
    virtual void EmitValueToAlignment(unsigned ByteAlignment, int64_t Value = 0,
                                      unsigned ValueSize = 1,
                                      unsigned MaxBytesToEmit = 0) = 0;

    /// EmitValueToOffset - Emit some number of copies of @param Value until the
    /// byte offset @param Offset is reached.
    ///
    /// This is used to implement assembler directives such as .org.
    ///
    /// @param Offset - The offset to reach.This may be an expression, but the
    /// expression must be associated with the current section.
    /// @param Value - The value to use when filling bytes.
    // 
    // FIXME: How are we going to signal failures out of this?
    virtual void EmitValueToOffset(const MCValue &Offset, 
                                   unsigned char Value = 0) = 0;
    
    /// @}

    /// EmitInstruction - Emit the given @param Instruction into the current
    /// section.
    virtual void EmitInstruction(const MCInst &Inst) = 0;

    /// Finish - Finish emission of machine code and flush any output.
    virtual void Finish() = 0;
  };

  /// createAsmStreamer - Create a machine code streamer which will print out
  /// assembly for the native target, suitable for compiling with a native
  /// assembler.
  MCStreamer *createAsmStreamer(MCContext &Ctx, raw_ostream &OS);

  // FIXME: These two may end up getting rolled into a single
  // createObjectStreamer interface, which implements the assembler backend, and
  // is parameterized on an output object file writer.

  /// createMachOStream - Create a machine code streamer which will generative
  /// Mach-O format object files.
  MCStreamer *createMachOStreamer(MCContext &Ctx, raw_ostream &OS);

  /// createELFStreamer - Create a machine code streamer which will generative
  /// ELF format object files.
  MCStreamer *createELFStreamer(MCContext &Ctx, raw_ostream &OS);

} // end namespace llvm

#endif
