/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


// deleteStrux-related functions for class pt_PieceTable.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"

/****************************************************************/
/****************************************************************/
bool pt_PieceTable::_unlinkStrux(pf_Frag_Strux * pfs,
									pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	switch (pfs->getStruxType())	
	{
	case PTX_Section:
		return _unlinkStrux_Section(pfs,ppfEnd,pfragOffsetEnd);

	case PTX_SectionHdrFtr:
		return _unlinkStrux_Section(pfs,ppfEnd,pfragOffsetEnd);
		
	case PTX_Block:
		return _unlinkStrux_Block(pfs,ppfEnd,pfragOffsetEnd);
		
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
}

bool pt_PieceTable::_unlinkStrux_Block(pf_Frag_Strux * pfs,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(pfs->getStruxType()==PTX_Block);
	
	// unlink this Block strux from the document.
	// the caller is responsible for deleting pfs.

	if (ppfEnd)
		*ppfEnd = pfs->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	// find the previous strux (either a paragraph or something else).

	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag * pf = pfs->getPrev();
	while (pf && !pfsPrev)
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfsPrev = static_cast<pf_Frag_Strux *> (pf);
		pf = pf->getPrev();
	}
	UT_ASSERT(pfsPrev);			// we have a block that's not in a section ??

	switch (pfsPrev->getStruxType())
	{
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// paragraph knowing that our content will be assimilated
		// in to the previous one.

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_Section:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return false;
		}


	case PTX_SectionHdrFtr:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return false;
		}

		// no content in this paragraph.
		
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool pt_PieceTable::_unlinkStrux_Section(pf_Frag_Strux * pfs,
											pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(pfs->getStruxType()==PTX_Section || pfs->getStruxType()==PTX_SectionHdrFtr );
	
	// unlink this Section strux from the document.
	// the caller is responsible for deleting pfs.

	if (ppfEnd)
		*ppfEnd = pfs->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	// find the previous strux (either a paragraph or something else).

	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag * pf = pfs->getPrev();
	while (pf && !pfsPrev)
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfsPrev = static_cast<pf_Frag_Strux *> (pf);
		pf = pf->getPrev();
	}

	if (!pfsPrev)
	{
		// first section in the document cannot be deleted.
		// TODO decide if this should assesrt or just file...
		UT_DEBUGMSG(("Cannot delete first section in document.\n"));
		UT_ASSERT(0);
		return false;
	}
	
	switch (pfsPrev->getStruxType())
	{
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// section knowing that our paragraphs will be assimilated
		// in to the previous section (that is, the container of
		// this block).

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_Section:
		// there are no blocks (paragraphs) between this section
		// and the previous section.  this is not possible.
		// TODO decide if this should assert or just fail...
		UT_DEBUGMSG(("No blocks between sections ??\n"));
		UT_ASSERT(0);
		return false;


	case PTX_SectionHdrFtr:
		// there are no blocks (paragraphs) between this section
		// and the previous section.  this is not possible.
		// TODO decide if this should assert or just fail...
		UT_DEBUGMSG(("No blocks between sections ??\n"));
		UT_ASSERT(0);
		return false;

	default:
		UT_ASSERT(0);
		return false;
	}
}
			
bool pt_PieceTable::_deleteStruxWithNotify(PT_DocPosition dpos,
											  pf_Frag_Strux * pfs,
											  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_DeleteStrux,
									dpos, pfs->getIndexAP(), pfs->getStruxType());
	UT_ASSERT(pcrs);

	if (!_unlinkStrux(pfs,ppfEnd,pfragOffsetEnd))
		return false;
	
	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfs,pcrs);

	delete pfs;

	return true;
}

			
bool pt_PieceTable::_deleteStrux_norec(PT_DocPosition dpos,
											  pf_Frag_Strux * pfs,
											  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_DeleteStrux,
									dpos, pfs->getIndexAP(), pfs->getStruxType());
	UT_ASSERT(pcrs);

	if (!_unlinkStrux(pfs,ppfEnd,pfragOffsetEnd))
		return false;
	
	// No history for field updates..
	// m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfs,pcrs);

	delete pfs;

	return true;
}

/*!
 * This method scans the piecetAble from the section Frag_strux given looking 
 * for any Header/Footers that belong to the strux. If it finds them, they
 * are deleted with notifications.
\params pf_Frag_Strux_Section pfStruxSec the Section strux that might have headers
 *                                        or footers belonging to it.
 * These must be deleted with notification otherwise they won't be recreated on
 * an undo
 */
bool pt_PieceTable::_deleteHdrFtrsFromSectionStruxIfPresent(pf_Frag_Strux_Section * pfStruxSec)
{
	//
	// Get the index to the Attributes/properties of the section strux to see if 
	// if there is a header defined for this strux.
	//
	PT_AttrPropIndex indexAP = pfStruxSec->getIndexAP();
	const PP_AttrProp * pAP = NULL;
	getAttrProp(indexAP, &pAP);
	const char * szHeaderV = NULL;
	bool bres = pAP->getAttribute("header",szHeaderV);
	const char * szFooterV = NULL;
    bres = bres & pAP->getAttribute("footer",szFooterV);
	pf_Frag * curFrag = NULL;
	UT_DEBUGMSG(("SEVIOR: Deleting HdrFtrs from Document, headerID = %s footerID = %s \n",szHeaderV,szFooterV));
	if((szFooterV == NULL) && (szHeaderV == NULL))
	{
		return true;
	}
//
// This section has a header or footer attribute. Scan the piecetable to see 
// if there is a header strux somewhere with an ID that matches our section.
//
	curFrag = static_cast<pf_Frag *>(pfStruxSec);
	bool bFoundIt = false;
	pf_Frag_Strux * curStrux = NULL;
	getFragments().cleanFrags(); // clean up to be safe...
//
// Do this loop for both headers and footers seperately coz unlinking the header/footer content
// screws up the loop.
//

	while(curFrag != getFragments().getLast() && !bFoundIt)
	{
		if(curFrag->getType() == pf_Frag::PFT_Strux)
		{
			curStrux = static_cast<pf_Frag_Strux *>(curFrag);
			if(curStrux->getStruxType() == PTX_SectionHdrFtr)
			{
//
// OK we've got a candidate
//
				PT_AttrPropIndex indexAPHdr = curStrux->getIndexAP();
				const PP_AttrProp * pAPHdr = NULL;
				getAttrProp(indexAPHdr, &pAPHdr);
				const char * szID = NULL;
				bres = pAPHdr->getAttribute("id",szID);
				UT_DEBUGMSG(("SEVIOR: Found candidate id = %s \n",szID));
				if(bres && (szID != NULL))
				{
					//
					// Look for a match.
					//
					if(szHeaderV != NULL && strcmp(szHeaderV,szID) == 0)
					{
						bFoundIt = true;
					}
				}
			}
		}
		curFrag = curFrag->getNext();
	}
	if(bFoundIt)
	{
		//
		// This Header belongs to our section. It must be deleted.
		//
		_deleteHdrFtrStruxWithNotify(curStrux);
	}
//
// Now the footer.
// 
	curFrag = static_cast<pf_Frag *>(pfStruxSec);
	bFoundIt = false;
//
// Clean up after deleting the text in the Header.
//
	getFragments().cleanFrags();
	while(curFrag != getFragments().getLast() && !bFoundIt)
	{
		if(curFrag->getType() == pf_Frag::PFT_Strux)
		{
			curStrux = static_cast<pf_Frag_Strux *>(curFrag);
			if(curStrux->getStruxType() == PTX_SectionHdrFtr)
			{
//
// OK we've got a candidate
//
				PT_AttrPropIndex indexAPHdr = curStrux->getIndexAP();
				const PP_AttrProp * pAPHdr = NULL;
				getAttrProp(indexAPHdr, &pAPHdr);
				const char * szID = NULL;
				bres = pAPHdr->getAttribute("id",szID);
				UT_DEBUGMSG(("SEVIOR: Found a footer candidate id = %s \n",szID));
				if(bres && (szID != NULL))
				{
					//
					// Look for a match.
					//
					if(szFooterV != NULL && strcmp(szFooterV,szID) == 0)
					{
						bFoundIt = true;
					}
				}
			}
		}
		curFrag = curFrag->getNext();
	}
	if(bFoundIt)
	{
		//
		// This Footer belongs to our section. It must be deleted.
		//
		_deleteHdrFtrStruxWithNotify(curStrux);
	}
	getFragments().cleanFrags(); // clean up to be safe...
	return true;
}

/*!
 * This method deletes the Header/Footer from the pieceTable in the order that
 * will allow an undo to recreate it.
 */
void pt_PieceTable::_deleteHdrFtrStruxWithNotify( pf_Frag_Strux * pfFragStruxHdrFtr)
{
//
// First we need the document position of the header/footer strux.
// 
	UT_DEBUGMSG(("SEVIOR: Deleting hdrftr \n"));
	const pf_Frag * pfFrag = NULL;
	pfFrag = static_cast<pf_Frag *>(pfFragStruxHdrFtr);
	PT_DocPosition HdrFtrPos = getFragPosition(pfFrag);
	UT_DEBUGMSG(("SEVIOR: Deleting hdrftr Strux Pos = %d \n",HdrFtrPos));
//
// Now find the first Non-strux frag.
//
	while((pfFrag->getType() == pf_Frag::PFT_Strux) && (pfFrag != getFragments().getLast()))
	{
		pfFrag = pfFrag->getNext();
	}
	PT_DocPosition TextStartPos = getFragPosition(pfFrag);
	UT_DEBUGMSG(("SEVIOR: Deleting hdrftr Text Start Pos = %d \n",TextStartPos));
//
// Now find the end of the text in the header/footer
//
	bool foundEnd = false;
	while(!foundEnd)
	{
		foundEnd = pfFrag == getFragments().getLast();
		if(!foundEnd && pfFrag->getType() == pf_Frag::PFT_Strux)
		{
			const pf_Frag_Strux * pfFragStrux = static_cast<const pf_Frag_Strux *>(pfFrag);
			foundEnd = pfFragStrux->getStruxType() != PTX_Block;
		}
		if(!foundEnd)
		{
			pfFrag = pfFrag->getNext();
		}
	}
	PT_DocPosition TextEndPos = getFragPosition(pfFrag);
	UT_DEBUGMSG(("SEVIOR: Deleting hdrftr Text End Pos = %d \n",TextEndPos));
//
// OK delete the text
//
	if(TextEndPos > TextStartPos)
	{
		deleteSpan(TextStartPos,TextEndPos,NULL,true);
	}
//
// Now delete the struxes at the start.
//
	deleteSpan(HdrFtrPos,TextStartPos,NULL,true);
}




