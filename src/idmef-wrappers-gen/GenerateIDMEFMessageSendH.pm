# Copyright (C) 2003 Nicolas Delon <delon.nicolas@wanadoo.fr>
# All Rights Reserved
#
# This file is part of the Prelude program.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

package GenerateIDMEFMessageSendH;

use Generate;
@ISA = qw/Generate/;

use strict;
use IDMEFTree;

sub	header
{
     my	$self = shift;
     my	$time = localtime;

     $self->output("
/* Auto-generated by the GenerateIDMEFMessageSendH package ($time) */

#ifndef _LIBPRELUDE_IDMEF_MESSAGE_SEND_H
#define _LIBPRELUDE_IDMEF_MESSAGE_SEND_H

void idmef_send_time(prelude_msgbuf_t *msg, uint8_t tag, idmef_time_t *data);
");
}

sub	footer
{
    my	$self = shift;

    $self->output("
#endif /* _LIBPRELUDE_IDMEF_MESSAGE_SEND_H */
");
}

sub	struct
{
    my	$self = shift;
    my	$tree = shift;
    my	$struct = shift;

    $self->output("void idmef_send_$struct->{short_typename}(prelude_msgbuf_t *msg, $struct->{typename} *$struct->{short_typename});\n");
}

1;
