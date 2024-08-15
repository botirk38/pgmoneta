/*
 * Copyright (C) 2024 The pgmoneta community
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may
 * be used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* pgmoneta */
#include <art.h>
#include <deque.h>
#include <json.h>
#include <utils.h>
#include <value.h>
#include <verify.h>

/* System */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void noop_destroy_cb(uintptr_t data);
static void free_destroy_cb(uintptr_t data);
static void art_destroy_cb(uintptr_t data);
static void deque_destroy_cb(uintptr_t data);
static void json_destroy_cb(uintptr_t data);
static char* noop_to_string_cb(uintptr_t data, char* tag, int indent);
static char* int8_to_string_cb(uintptr_t data, char* tag, int indent);
static char* uint8_to_string_cb(uintptr_t data, char* tag, int indent);
static char* int16_to_string_cb(uintptr_t data, char* tag, int indent);
static char* uint16_to_string_cb(uintptr_t data, char* tag, int indent);
static char* int32_to_string_cb(uintptr_t data, char* tag, int indent);
static char* uint32_to_string_cb(uintptr_t data, char* tag, int indent);
static char* int64_to_string_cb(uintptr_t data, char* tag, int indent);
static char* uint64_to_string_cb(uintptr_t data, char* tag, int indent);
static char* float_to_string_cb(uintptr_t data, char* tag, int indent);
static char* double_to_string_cb(uintptr_t data, char* tag, int indent);
static char* string_to_string_cb(uintptr_t data, char* tag, int indent);
static char* bool_to_string_cb(uintptr_t data, char* tag, int indent);
static char* deque_to_string_cb(uintptr_t data, char* tag, int indent);
static char* art_to_string_cb(uintptr_t data, char* tag, int indent);
static char* json_to_string_cb(uintptr_t data, char* tag, int indent);
static char* verify_entry_to_string_cb(uintptr_t data, char* tag, int indent);

int
pgmoneta_value_create(enum value_type type, uintptr_t data, struct value** value)
{
   struct value* val = NULL;
   val = (struct value*) malloc(sizeof(struct value));
   if (val == NULL)
   {
      goto error;
   }
   val->data = 0;
   val->type = type;
   switch (type)
   {
      case ValueInt8:
         val->to_string = int8_to_string_cb;
         break;
      case ValueUInt8:
         val->to_string = uint8_to_string_cb;
         break;
      case ValueInt16:
         val->to_string = int16_to_string_cb;
         break;
      case ValueUInt16:
         val->to_string = uint16_to_string_cb;
         break;
      case ValueInt32:
         val->to_string = int32_to_string_cb;
         break;
      case ValueUInt32:
         val->to_string = uint32_to_string_cb;
         break;
      case ValueInt64:
         val->to_string = int64_to_string_cb;
         break;
      case ValueUInt64:
         val->to_string = uint64_to_string_cb;
         break;
      case ValueFloat:
         val->to_string = float_to_string_cb;
         break;
      case ValueDouble:
         val->to_string = double_to_string_cb;
         break;
      case ValueBool:
         val->to_string = bool_to_string_cb;
         break;
      case ValueString:
         val->to_string = string_to_string_cb;
         break;
      case ValueJSON:
         val->to_string = json_to_string_cb;
         break;
      case ValueDeque:
         val->to_string = deque_to_string_cb;
         break;
      case ValueART:
         val->to_string = art_to_string_cb;
         break;
      case ValueVerifyEntry:
         val->to_string = verify_entry_to_string_cb;
         break;
      default:
         val->to_string = noop_to_string_cb;
         break;
   }
   switch (type)
   {
      case ValueString:
      {
         char* orig = NULL;
         char* str = NULL;

         orig = (char*) data;
         if (orig != NULL)
         {
            str = pgmoneta_append(str, orig);
         }

         val->data = (uintptr_t) str;
         val->destroy_data = free_destroy_cb;
         break;
      }
      case ValueVerifyEntry:
         val->data = data;
         val->destroy_data = free_destroy_cb;
         break;
      case ValueJSON:
         val->data = data;
         val->destroy_data = json_destroy_cb;
         break;
      case ValueDeque:
         val->data = data;
         val->destroy_data = deque_destroy_cb;
         break;
      case ValueART:
         val->data = data;
         val->destroy_data = art_destroy_cb;
         break;
      default:
         val->data = data;
         val->destroy_data = noop_destroy_cb;
         break;
   }
   *value = val;
   return 0;

error:
   return 1;
}

int
pgmoneta_value_destroy(struct value* value)
{
   if (value == NULL)
   {
      return 0;
   }
   value->destroy_data(value->data);
   free(value);
   return 0;
}

uintptr_t
pgmoneta_value_data(struct value* value)
{
   if (value == NULL)
   {
      return 0;
   }
   return value->data;
}

char*
pgmoneta_value_to_string(struct value* value, char* tag, int indent)
{
   return value->to_string(value->data, tag, indent);
}

uintptr_t
pgmoneta_value_from_double(double val)
{
   union duni
   {
      double val;
      uintptr_t data;
   };
   union duni uni;
   uni.val = val;
   return uni.data;
}

double
pgmoneta_value_to_double(uintptr_t val)
{
   union duni
   {
      double val;
      uintptr_t data;
   };
   union duni uni;
   uni.data = val;
   return uni.val;
}

uintptr_t
pgmoneta_value_from_float(float val)
{
   union funi
   {
      float val;
      uintptr_t data;
   };
   union funi uni;
   uni.val = val;
   return uni.data;
}

float
pgmoneta_value_to_float(uintptr_t val)
{
   union funi
   {
      float val;
      uintptr_t data;
   };
   union funi uni;
   uni.data = val;
   return uni.val;
}

static void
noop_destroy_cb(uintptr_t data)
{
   (void) data;
}

static void
free_destroy_cb(uintptr_t data)
{
   free((void*) data);
}

static void
art_destroy_cb(uintptr_t data)
{
   pgmoneta_art_destroy((struct art*) data);
}

static void
deque_destroy_cb(uintptr_t data)
{
   pgmoneta_deque_destroy((struct deque*) data);
}

static void
json_destroy_cb(uintptr_t data)
{
   pgmoneta_json_free((struct json*) data);
}

static char*
noop_to_string_cb(uintptr_t data, char* tag, int indent)
{
   (void) data;
   (void) tag;
   (void) indent;
   return NULL;
}

static char*
int8_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRId8, (int8_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
uint8_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRIu8, (uint8_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
int16_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRId16, (int16_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
uint16_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRIu16, (uint16_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
int32_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRId32, (int32_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
uint32_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRIu32, (uint32_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
int64_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRId64, (int64_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
uint64_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%" PRIu64, (uint64_t)data);
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
float_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%f", pgmoneta_value_to_float(data));
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
double_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   snprintf(buf, MISC_LENGTH, "%f", pgmoneta_value_to_double(data));
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
string_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   char* str = (char*) data;
   char buf[MISC_LENGTH];
   memset(buf, 0, MISC_LENGTH);
   if (str == NULL)
   {
      snprintf(buf, MISC_LENGTH, "null");
   }
   else
   {
      snprintf(buf, MISC_LENGTH, "\"%s\"", str);
   }
   ret = pgmoneta_append(ret, buf);
   return ret;
}

static char*
bool_to_string_cb(uintptr_t data, char* tag, int indent)
{
   char* ret = NULL;
   ret = pgmoneta_indent(ret, tag, indent);
   bool val = (bool) data;
   ret = pgmoneta_append(ret, val?"true":"false");
   return ret;
}

static char*
deque_to_string_cb(uintptr_t data, char* tag, int indent)
{
   return pgmoneta_deque_to_string((struct deque*)data, tag, indent);
}

static char*
art_to_string_cb(uintptr_t data, char* tag, int indent)
{
   return pgmoneta_art_to_string((struct art*) data, tag, indent);
}

static char*
json_to_string_cb(uintptr_t data, char* tag, int indent)
{
   return pgmoneta_json_to_string((struct json*)data, tag, indent);
}

static char*
verify_entry_to_string_cb(uintptr_t data, char* tag, int indent)
{
   return pgmoneta_verify_entry_to_string((struct verify_entry*)data, tag, indent);
}