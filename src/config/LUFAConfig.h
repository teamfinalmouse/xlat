/*
 * Copyright (c) 2023 Finalmouse, LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _LUFA_CONFIG_H_
#define _LUFA_CONFIG_H_

    #if (ARCH == ARCH_CUSTOM)
        #define HID_PARSER_STREAM_ONLY  // Do not keep all reports in memory, stream them through CALLBACK_HIDParser_FilterHIDReportItem
        #define HID_MAX_COLLECTIONS     20
    #else
		#error Unsupported architecture for this LUFA configuration file.
    #endif

#endif
