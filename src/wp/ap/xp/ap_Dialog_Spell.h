/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#ifndef AP_DIALOG_SPELL_H
#define AP_DIALOG_SPELL_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"

#include "sp_spell.h"

class XAP_Frame;

class AP_Dialog_Spell : public XAP_Dialog_NonPersistent
{
   
 public:

   AP_Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
   virtual ~AP_Dialog_Spell(void);
   
   virtual void runModal(XAP_Frame * pFrame) = 0;

   UT_Bool isComplete(void) const { return !m_bCancelled; };

 protected:

   void _purgeSuggestions(void);

   UT_UCSChar * _getCurrentWord(void);
   UT_UCSChar * _getPreWord(void);
   UT_UCSChar * _getPostWord(void);
     
   void _updateSentenceBoundaries(void);
   UT_uint32 m_iSentenceStart;
   UT_uint32 m_iSentenceEnd;
   
   PT_DocPosition m_iOrigInsPoint;
   
   // used to find misspelled words
   UT_Bool nextMisspelledWord(void);
  
   UT_Bool addIgnoreAll(void);
   void ignoreWord(void);
   
   UT_Bool inChangeAll(void);
   UT_Bool addChangeAll(UT_UCSChar * newword);
   UT_Bool changeWordWith(UT_UCSChar * newword);

   // make the word visible in the document behind the dialog
   UT_Bool makeWordVisible(void);
   // add the word to current user dictionary
   UT_Bool addToDict(void);

   // change/ignore all hash tables
   UT_HashTable * m_pChangeAll;
   UT_HashTable * m_pIgnoreAll;
   
   // these variables keep track of the current
   // location/state of the search through the
   // document for misspelled words
   UT_uint32 m_iWordOffset;
   UT_sint32 m_iWordLength;
   UT_GrowBuf * m_pBlockBuf;

   fl_DocSectionLayout * m_pSection;
   fl_BlockLayout * m_pBlock;
   
   XAP_Frame * m_pFrame;
   FV_View * m_pView;
   PD_Document * m_pDoc;
   
   // current suggested corrections to the 
   // most recently misspelled word
   sp_suggestions m_Suggestions;

   UT_Bool	m_bCancelled;
   short m_iSelectedRow;
   
};

#endif /* AP_DIALOG_SPELL_H */
