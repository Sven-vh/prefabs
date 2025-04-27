/**
   dtl -- Diff Template Library

   In short, Diff Template Library is distributed under so called "BSD license",

   Copyright (c) 2015 Tatsuhiko Kubo <cubicdaiya@gmail.com>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   * Neither the name of the authors nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* If you use this library, you must include dtl.hpp only. */

#ifndef DTL_SES_H
#define DTL_SES_H

namespace dtl {

	/**
	 * Shortest Edit Script template class
	 */
	template <typename elem>
	class Ses : public Sequence< elem > {
	private:
		using sesElem = std::pair< elem, elemInfo >;
		using sesSeq = std::list< sesElem >;
	public:

		Ses() : onlyAdd(true), onlyDelete(true), onlyCopy(true), deletesFirst(false) {
			nextDeleteIt = sequence.begin();

		}
		Ses(bool moveDel) : onlyAdd(true), onlyDelete(true), onlyCopy(true), deletesFirst(moveDel) {
			nextDeleteIt = sequence.begin();

		}
		~Ses() {}

		bool isOnlyAdd() const {
			return onlyAdd;
		}

		bool isOnlyDelete() const {
			return onlyDelete;
		}

		bool isOnlyCopy() const {
			return onlyCopy;
		}

		bool isOnlyOneOperation() const {
			return isOnlyAdd() || isOnlyDelete() || isOnlyCopy();
		}

		bool isChange() const {
			return !onlyCopy;
		}

		using Sequence< elem >::addSequence;
		void addSequence(elem e, long long beforeIdx, long long afterIdx, const edit_t type) {
			elemInfo info;
			info.beforeIdx = beforeIdx;
			info.afterIdx = afterIdx;
			info.type = type;
			sesElem pe(e, info);
			if (!deletesFirst)
				sequence.push_back(pe);
			switch (type) {
			case SES_DELETE:
				onlyCopy = false;
				onlyAdd = false;
				if (deletesFirst) {
					// insert-before nextDeleteIt, returns iterator to new element
					auto justInserted = sequence.insert(nextDeleteIt, pe);
					// advance nextDeleteIt past the newly-inserted node
					nextDeleteIt = std::next(justInserted);

				}
				break;
			case SES_COMMON:
				onlyAdd = false;
				onlyDelete = false;
				if (deletesFirst) {
					sequence.push_back(pe);
					// all deletes done; move iterator to the end
					nextDeleteIt = sequence.end();

				}
				break;
			case SES_ADD:
				onlyDelete = false;
				onlyCopy = false;
				if (deletesFirst)
					sequence.push_back(pe);
				break;
			}
		}

		// now returns a const ref to the list
		const sesSeq& getSequence() const {
			return sequence;

		}
	private:
		sesSeq     sequence;
		bool       onlyAdd;
		bool       onlyDelete;
		bool       onlyCopy;
		bool       deletesFirst;
		// instead of an index, track an insertion-point iterator
		typename sesSeq::iterator nextDeleteIt;
	};
}

#endif // DTL_SES_H
