/*----------------------------------------------------------------------------*-

   ttrd2-05a-t0401a-v001a_reg_conf_chk_timer.h (Release 2017-03-06a)

  ----------------------------------------------------------------------------
   
   - See ttrd2-05a-t0401a-v001a_reg_conf_chk_timer.c for details.

-*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*-

  This code is copyright (c) 2014-2017 SafeTTy Systems Ltd.

  This code forms part of a Time-Triggered Reference Design (TTRD) that is 
  documented in the following book: 

   Pont, M.J. (2016) 
   "The Engineering of Reliable Embedded Systems (Second Edition)", 
   Published by SafeTTy Systems Ltd. ISBN: 978-0-9930355-3-1.

  Both the TTRDs and the above book ("ERES2") describe patented 
  technology and are subject to copyright and other restrictions.

  This code may be used without charge: [i] by universities and colleges on 
  courses for which a degree up to and including 'MSc' level (or equivalent) 
  is awarded; [ii] for non-commercial projects carried out by individuals 
  and hobbyists.

  Any and all other use of this code and / or the patented technology 
  described in ERES2 requires purchase of a ReliabiliTTy Technology Licence:
  http://www.safetty.net/technology/reliabilitty-technology-licences

  Please contact SafeTTy Systems Ltd if you require clarification of these 
  licensing arrangements: http://www.safetty.net/contact

  CorrelaTTor, DecomposiTTor, DuplicaTTor, MoniTTor, PredicTTor, ReliabiliTTy,  
  SafeTTy, SafeTTy Systems, TriplicaTTor and WarranTTor are registered 
  trademarks or trademarks of SafeTTy Systems Ltd in the UK & other countries.

-*----------------------------------------------------------------------------*/

#ifndef _TIMER_CHECKS_H
#define _TIMER_CHECKS_H 1

// ------ Public function prototypes (declarations) --------------------------

void REG_CONFIG_TIMER_Store(TIM_TypeDef* TIMx);
void REG_CONFIG_TIMER_Check(TIM_TypeDef* TIMx);

#endif

/*----------------------------------------------------------------------------*-
  ------------------------------ END OF FILE ---------------------------------
-*----------------------------------------------------------------------------*/

