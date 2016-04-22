/**
 * \file
 * \brief Provides the definition for the remove double indirection exp
 *        tranformer.
 *
 * \authors
 * Copyright (C) 2004, Trent Waddington
 */

#ifndef REMOVE_DOUBLE_INDIRECTION_H
#define REMOVE_DOUBLE_INDIRECTION_H

class RDIExpTransformer : public ExpTransformer
{
public:
	RDIExpTransformer() {}
	virtual Exp *applyTo(Exp *e, bool &bMod);
};

#endif
