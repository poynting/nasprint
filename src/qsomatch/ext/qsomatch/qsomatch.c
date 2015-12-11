/***************************************************************************
 * Name: qsomatch.c
 * Code to store a QSO and make a probabilistic match.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <ctype.h>
#include "ruby.h"
#include "distance.h"
#include "adj_matrix.h"
#include "svmmodel.h"

#define MAX_CALLSIGN_CHARS   12
#define MAX_MULTIPLIER_CHARS 20

struct svm_model *s_svm_model = 0;

/* order must agree with s_bandName */
enum Band_t {
  b_OnePointTwoG,
  b_TenG,
  b_TenM,
  b_OneNineteenG,
  b_TwelveM,
  b_OneFortyTwoG,
  b_FifteenM,
  b_OneSixtyM,
  b_SeventeenM,
  b_TwoPointThreeG,
  b_TwentyM,
  b_TwoTwentyTwoM,
  b_TwoFortyOneG,
  b_TwentyFourG,
  b_TwoM,
  b_ThreePointFourG,
  b_ThirtyM,
  b_FortyM,
  b_FourThirtyTwoM,
  b_FortySevenG,
  b_FivePointSevenG,
  b_SixM,
  b_SeventyFiveG,
  b_EightyM,
  b_NineZeroTwoM,
  b_Unknown
};

#define MAX_BAND_NAME 8
/* order must match ordering of enum above */
static const char * const s_bandNames[] = {
  "1.2G",
  "10G",
  "10m",
  "119G",
  "12m",
  "142G",
  "15m",
  "160m",
  "17m",
  "2.3G",
  "20m",
  "222",
  "241G",
  "24G",
  "2m",
  "3.4G",
  "30m",
  "40m",
  "432",
  "47G",
  "5.7G",
  "6m",
  "75G",
  "80m",
  "902",
  "unknown"
};

struct StringMap_t {
  const char d_bandName[MAX_BAND_NAME];
  const int  d_bandNum;
};

static struct StringMap_t s_bandMap[] = {
  { "1.2G", b_OnePointTwoG},
  { "10G",  b_TenG},
  { "10m",  b_TenM},
  { "119G", b_OneNineteenG},
  { "12m",  b_TwelveM},
  { "142G", b_OneFortyTwoG},
  { "15m",  b_FifteenM},
  { "160m", b_OneSixtyM},
  { "17m",  b_SeventeenM},
  { "2.3G", b_TwoPointThreeG},
  { "20m",  b_TwentyM},
  { "222",  b_TwoTwentyTwoM},
  { "241G", b_TwoFortyOneG},
  { "24G",  b_TwentyFourG},
  { "2m",   b_TwoM},
  { "3.4G", b_ThreePointFourG},
  { "30m",  b_ThirtyM},
  { "40m",  b_FortyM},
  { "432",  b_FourThirtyTwoM},
  { "47G",  b_FortySevenG},
  { "5.7G", b_FivePointSevenG},
  { "6m",   b_SixM},
  { "75G",  b_SeventyFiveG},
  { "80m",  b_EightyM},
  { "902",  b_NineZeroTwoM},
  { "unknown", b_Unknown}
};

enum Mode_t {
  m_Phone,
  m_CW,
  m_FM,
  m_RTTY
};

static
const char * const s_modeNames[] = {
  "PH",
  "CW",
  "FM",
  "RY"
};

static struct StringMap_t s_modeMap[] = {
  { "CW", m_CW },
  { "FM", m_FM },
  { "PH", m_Phone },
  { "RY", m_RTTY }
};

struct Exchange_t {
  int16_t d_serial;			  /* serial number */
  char    d_callsign[MAX_CALLSIGN_CHARS]; /* callsign as logged */
  char    d_basecall[MAX_CALLSIGN_CHARS]; /* callsign with prefix/suffix removed */
  char    d_multiplier[MAX_MULTIPLIER_CHARS]; /* canonical multiplier name */
  char    d_location[MAX_MULTIPLIER_CHARS];   /* multiplier name as logged */
};

struct QSO_t {
  int32_t           d_qsoID;	/* unique number assigned to each QSO */
  int32_t           d_logID;	/* unique number assigned to each LOG */
  int32_t	    d_frequency;/* frequency number as logged */
  enum Band_t       d_band;
  enum Mode_t       d_mode;
  time_t            d_datetime;	/* date and time of QSO as seconds since epoch */
  struct Exchange_t d_sent;
  struct Exchange_t d_recvd;
};

const char * const s_CW_MAPPING[] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  " ",				/* space */
  "-.-.--",			/* ! exclamation point */
  ".-..-.",			/* " quotation mark */
  0,				/* # hash */
  "...-..-",			/* $ dollar sign */
  0,				/* % percent */
  ".-...",			/* & ampersand */
  0,				/* ' single quote */
  "-.--.",			/* ( open paren */
  "-.--.-",			/* ) close paren */
  0,				/* * asterisk */
  ".-.-.",			/* + plus */
  "--..--",			/* , comma */
  "-....-",			/* - hyphen */
  ".-.-.-",			/* . period */
  "-..-.",			/* / slash */
  "-----",			/* 0 zero */
  ".----",			/* 1 one */
  "..---",			/* 2 two */
  "...--",			/* 3 three */
  "....-",			/* 4 four */
  ".....",			/* 5 five */
  "-....",			/* 6 six */
  "--...",			/* 7 seven */
  "---..",			/* 8 eight */
  "----.",			/* 9 nine */
  "---...",			/* : colon */
  "-.-.-.",			/* ; semicolon */
  0,				/* < less than */
  "-...-",			/* = double dash */
  0,				/* > greater than */
  "..--..",			/* ? question mark */
  ".--.-.",			/* @ at */
  ".-",  			/* A */
  "-...",			/* B */
  "-.-.",			/* C */
  "-..",			/* D */
  ".",				/* E */
  "..-.",			/* F */
  "--.",			/* G */
  "...",			/* H */
  "..",				/* I */
  ".---",			/* J */
  "-.-",			/* K */
  ".-..",			/* L */
  "--",				/* M */
  "-.",				/* N */
  "---",			/* O */
  ".--.",			/* P */
  "--.-",			/* Q */
  ".-.",			/* R */
  "...",			/* S */
  "-",				/* T */
  "..-",			/* U */
  "...-",			/* V */
  ".--",			/* W */
  "-..-",			/* X */
  "-.--",			/* Y */
  "--..",			/* Z */
  0,				/* [ */
  0,				/* \ */
  0,				/* ] */
  0,				/* ^ */
  "..--.-"			/* _ underscore */
};

static VALUE rb_cQSO, rb_eQSOError;

static jw_AdjMatrix *s_CW_adj = 0;
static jw_AdjMatrix *s_Normal_adj = 0;

static void
free_qso(void *ptr);

static size_t
memsize_qso(const void *ptr);

static const rb_data_type_t qso_type = {
  "qso",
  { 0, free_qso, memsize_qso,},
  0, 0,
  RUBY_TYPED_FREE_IMMEDIATELY,
};

static void
freed_qso(void)
{
  rb_raise(rb_eQSOError, "deallocated QSO");
}

static VALUE
qso_s_allocate(VALUE klass)
{
  struct QSO_t *qsop;
  VALUE result =  TypedData_Make_Struct(klass, struct QSO_t,
					&qso_type, qsop);
  memset(qsop, 0, sizeof(struct QSO_t));
  return result;
}

#define GetQSO(obj, qsop) do {\
  TypedData_Get_Struct((obj), struct QSO_t, &qso_type, (qsop));\
  if ((qsop) == 0) freed_qso();\
  } while(0)

static void
free_qso(void *ptr)
{
  struct QSO_t *qsop = (struct QSO_t *)ptr;
  if (qsop) {
    memset(qsop, 0, sizeof(struct QSO_t));
    xfree(qsop);
  }
}

static size_t
memsize_qso(const void *ptr)
{
  size_t size = 0;
  const struct QSO_t *qsop = (const struct QSO_t *)ptr;
  if (qsop) {
    size += sizeof(struct QSO_t);
  }
  return size;
}

/*
 * call-seq:
 *    qso.id
 *
 * Return the unique id of the QSO.
 */
static VALUE
qso_id(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return INT2FIX(qsop->d_qsoID);
}

/*
 * call-seq:
 *     qso.impossibleMatch?(q)
 *
 * Return true iff qso and q cannot possibly match (e.g., from same log or
 * QSO id of q is <= the QSO id of qso.
 */
static VALUE
qso_impossible(VALUE obj, VALUE q)
{
  struct QSO_t *qsop, *qp;
  GetQSO(obj, qsop);
  GetQSO(q, qp);
  return ((qp->d_qsoID <= qsop->d_qsoID ) || (qp->d_logID == qsop->d_logID))
    ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *    qso.logID
 *
 * Return the unique ID of the log this QSO appears in.
 */
static VALUE
qso_logID(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return INT2FIX(qsop->d_logID);
}

/*
 * call-seq:
 *    qso.freq
 *
 * Return the frequency number for this QSO.
 */
static VALUE
qso_freq(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return INT2FIX(qsop->d_frequency);
}

/*
 * call-seq:
 *    qso.band
 *
 * Return the band string for this QSO.
 */
static VALUE
qso_band(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  if (qsop->d_band < 0 ||
      qsop->d_band >= (sizeof(s_bandNames)/sizeof(char *)))
    rb_raise(rb_eQSOError, "QSO has an illegal band");
  return rb_str_new2(s_bandNames[qsop->d_band]);
}

/*
 * call-seq:
 *    qso.mode
 *
 * Return the mode string for this QSO.
 */
static VALUE
qso_mode(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  if (qsop->d_mode < 0 ||
      qsop->d_mode >= (sizeof(s_modeNames)/sizeof(char *)))
    rb_raise(rb_eQSOError, "QSO has an illegal mode");
  return rb_str_new2(s_modeNames[qsop->d_mode]);
}

/*
 * call-seq:
 *     qso.datetime
 *
 * Return the date and time of the QSO.
 */
static VALUE
qso_datetime(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return rb_time_new(qsop->d_datetime, 0L);
}

static VALUE
qso_exchange_basecall(const struct Exchange_t *e)
{
  return e->d_basecall[0] ? rb_str_new2(e->d_basecall) : Qnil;
}

static VALUE
qso_exchange_callsign(const struct Exchange_t *e)
{
  return e->d_callsign[0] ? rb_str_new2(e->d_callsign) : Qnil;
}

static VALUE
qso_exchange_serial(const struct Exchange_t *e)
{
  return (e->d_serial >= 0) ? INT2FIX(e->d_serial) : Qnil;
}

static VALUE
qso_exchange_multiplier(const struct Exchange_t *e)
{
  return e->d_multiplier[0] ? rb_str_new2(e->d_multiplier) : Qnil;
}

static VALUE
qso_exchange_location(const struct Exchange_t *e)
{
  return e->d_location[0] ? rb_str_new2(e->d_location) : Qnil;
}

/*
 * call-seq:
 *     qso.recvd_basecall
 *
 * Return the base callsign of the received exchange.
 */
static VALUE
qso_recvd_basecall(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_basecall(&(qsop->d_recvd));
}

/*
 * call-seq:
 *     qso.recvd_callsign
 *
 * Return the logged callsign of the received exchange.
 */
static VALUE
qso_recvd_callsign(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_callsign(&(qsop->d_recvd));
}

/*
 * call-seq:
 *     qso.recvd_serial
 *
 * Return the serial number of the received exchange.
 */
static VALUE
qso_recvd_serial(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_serial(&(qsop->d_recvd));
}

/*
 * call-seq:
 *     qso.recvd_multiplier
 *
 * Return the multiplier of the received exchange.
 */
static VALUE
qso_recvd_multiplier(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_multiplier(&(qsop->d_recvd));
}

/*
 * call-seq:
 *     qso.recvd_location
 *
 * Return the logged location of the received exchange.
 */
static VALUE
qso_recvd_location(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_location(&(qsop->d_recvd));
}

/*
 * call-seq:
 *     qso.sent_basecall
 *
 * Return the base callsign of the sent exchange.
 */
static VALUE
qso_sent_basecall(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_basecall(&(qsop->d_sent));
}

/*
 * call-seq:
 *     qso.sent_callsign
 *
 * Return the logged callsign of the sent exchange.
 */
static VALUE
qso_sent_callsign(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_callsign(&(qsop->d_sent));
}

/*
 * call-seq:
 *     qso.sent_serial
 *
 * Return the serial number of the sent exchange.
 */
static VALUE
qso_sent_serial(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_serial(&(qsop->d_sent));
}

/*
 * call-seq:
 *     qso.sent_multiplier
 *
 * Return the multiplier of the sent exchange.
 */
static VALUE
qso_sent_multiplier(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_multiplier(&(qsop->d_sent));
}

/*
 * call-seq:
 *     qso.sent_location
 *
 * Return the logged location of the sent exchange.
 */
static VALUE
qso_sent_location(VALUE obj)
{
  struct QSO_t *qsop;
  GetQSO(obj, qsop);
  return qso_exchange_location(&(qsop->d_sent));
}

/*
 * call-seq:
 *     qso.basicLine
 *
 * Return a string that holds the QSO in a Cabrillo line format.
 */
static VALUE
qso_basicLine(VALUE obj)
{
  char buffer[71+4*(MAX_CALLSIGN_CHARS)+4*(MAX_MULTIPLIER_CHARS)];
  struct QSO_t *qsop;
  struct tm result;
  GetQSO(obj, qsop);
  gmtime_r(&(qsop->d_datetime), &result);
  snprintf(buffer,sizeof(buffer),
	   "%5d %-4s %-2s %04d-%02d-%02d %02d%02d %-7s %-7s %4d %-4s %-4s %-7s %-7s %4d %-4s %-4s",
	   qsop->d_frequency,
	   s_bandNames[qsop->d_band],
	   s_modeNames[qsop->d_mode],
	   1900+result.tm_year,
	   1+result.tm_mon,
	   result.tm_mday,
	   result.tm_hour,
	   result.tm_min,
	   qsop->d_sent.d_basecall,
	   qsop->d_sent.d_callsign,
	   qsop->d_sent.d_serial,
	   qsop->d_sent.d_multiplier,
	   qsop->d_sent.d_location,
	   qsop->d_recvd.d_basecall,
	   qsop->d_recvd.d_callsign,
	   qsop->d_recvd.d_serial,
	   qsop->d_recvd.d_multiplier,
	   qsop->d_recvd.d_location);
  return rb_str_new2(buffer);
}


static VALUE
q_to_s(const struct QSO_t *qsop,
     const struct Exchange_t *left,
     const struct Exchange_t *right)
{
  char buffer[88+4*(MAX_CALLSIGN_CHARS)+4*(MAX_MULTIPLIER_CHARS)];
  struct tm result;
  gmtime_r(&(qsop->d_datetime), &result);
  snprintf(buffer,sizeof(buffer),
	   "%7d %5d %5d %-4s %-2s %04d-%02d-%02d %02d%02d %-7s %-7s %4d %-4s %-4s %-7s %-7s %4d %-4s %-4s",
	   qsop->d_qsoID,
	   qsop->d_logID,
	   qsop->d_frequency,
	   s_bandNames[qsop->d_band],
	   s_modeNames[qsop->d_mode],
	   1900+result.tm_year,
	   1+result.tm_mon,
	   result.tm_mday,
	   result.tm_hour,
	   result.tm_min,
	   left->d_basecall,
	   left->d_callsign,
	   left->d_serial,
	   left->d_multiplier,
	   left->d_location,
	   right->d_basecall,
	   right->d_callsign,
	   right->d_serial,
	   right->d_multiplier,
	   right->d_location);
  return rb_str_new2(buffer);
}
    
/*
 * call-seq:
 *     qso.to_s(reverse=false)
 *
 * Return a string representation of the QSO. If reverse is true
 * the sent and received exchanges are reversed.
 */
static VALUE
qso_to_s(int argc, VALUE* argv, VALUE obj)
{
  struct QSO_t *qsop;
  VALUE reversed;
  int isReversed = 0;
  GetQSO(obj, qsop);
  if (rb_scan_args(argc, argv, "01", &reversed) == 1) {
    isReversed = RTEST(reversed);
  }
  if (isReversed) {
    return q_to_s(qsop, &(qsop->d_recvd), &(qsop->d_sent));
  }
 else {
    return q_to_s(qsop, &(qsop->d_sent), &(qsop->d_recvd));
 }
}


/* 
 * call-seq:
 *      qso.fullMatch?(qsob, time)
 *
 * Return true iff qso correctly logged QSO qsob to a given
 * time tolerance in minutes.  This does not require qsob to
 * have correctly received qso's exchange.
 */
static VALUE
qso_fullmatch(VALUE obj, VALUE qso, VALUE time)
{
  if (((T_FIXNUM == TYPE(time)) || (T_BIGNUM == TYPE(time))) &&
      (T_DATA == TYPE(qso))) {
    const struct QSO_t *selfp, *qsop;
    const long tolerance = NUM2LONG(time);
    GetQSO(obj, selfp);
    GetQSO(qso, qsop);
    return ((selfp != qsop) &&
	    ((selfp->d_band == qsop->d_band) &&
	     (selfp->d_mode == qsop->d_mode) &&
	     (labs((long)(selfp->d_datetime - qsop->d_datetime)) <=
	      tolerance * 60L) &&
	     (abs(selfp->d_recvd.d_serial - qsop->d_sent.d_serial) <= 1) &&
	     (0 == strcmp(selfp->d_recvd.d_basecall,
			  qsop->d_sent.d_basecall)) &&
	     (0 == strcmp(selfp->d_recvd.d_multiplier,
			  qsop->d_sent.d_multiplier))))
      ? Qtrue : Qfalse;
  }

  rb_raise(rb_eTypeError, "Incorrect arguments to fullMatch?");
}

static int
toCW(const char *src, char *dest)
{
  int result = 0;
  const size_t maxIndex = (char)(sizeof(s_CW_MAPPING)/sizeof(char *));
  while (*src) {
    const size_t index = (size_t)toupper(*src);
    if (index >= 0 && (index < maxIndex)) {
      const char * const asCW = s_CW_MAPPING[index];
      if (*asCW) {
	const size_t newlen = strlen(asCW);
	strcpy(dest, asCW);
	dest += newlen;
	result += newlen;
      }
    }
    else {
      *(dest++) = ' ';
      ++result;
    }
    ++src;
    if (*src) {
      *(dest++) = ' ';
      ++result;
    }
  }
  *dest = '\0';
  return result;
}

static VALUE
qso_toCW(VALUE cls, VALUE arg)
{
  if (T_STRING == TYPE(arg)) {
    const char *content = RSTRING_PTR(arg);
    const size_t len = RSTRING_LEN(arg);
    if (len <= 16) {
      char *buffer = malloc(8*len + 2);
      int resultLen = toCW(content, buffer);
      VALUE rresult = rb_str_new(buffer, resultLen);
      free(buffer);
      return rresult;
    }
  }
  rb_raise(rb_eTypeError, "Incorrect arguments to toCW (string required).");
  return Qnil;
}

static double
max_match(const char * const left[],  const int left_len,
	  const char * const right[], const int right_len,
	  const int isCW)
{
  double result = 0;
  jw_Option opt = jw_option_new();
  int i;
  opt.adj_table = (isCW ? s_CW_adj : s_Normal_adj);
  for(i = 0; i < left_len; ++i) {
    const int length_left = (int)strlen(left[i]);
    int j;
    for(j = 0; j < right_len; ++j) {
      const int length_right = (int)strlen(right[j]);
      double tmp = jw_distance(left[i], length_left,
			       right[j], length_right, opt);
      if (tmp > result) {
	result = tmp;
      }
    }
  }
  return result;
}

static int
pack_list(const char *s1, const char *s2,
	   const char *list[])
{
  int count = 0;
  if (s1)
    list[count++] = s1;
  if (s1 && s2 && strcmp(s1, s2)) {
    list[count++] = s2;
  }
  return count;
}

#define SERIAL_FULL 1
#define SERIAL_NONE 10

static double
serialNumberCmp(const int sent, const int recvd, const int isCW)
{
  jw_Option opt = jw_option_new();
  char buffer1[12], buffer2[12];
  int len1, len2;
  const int diff = abs(sent-recvd);
  double tmp, result = (diff > SERIAL_FULL)
    ? ((diff >= SERIAL_NONE) ? 0 : (1.0 - ((1.0*diff - SERIAL_FULL)/
					   (1.0*SERIAL_NONE-SERIAL_FULL))))
    : 1.0;
  opt.adj_table = (isCW ? s_CW_adj : s_Normal_adj);
  len1 = snprintf(buffer1, sizeof(buffer1), "%d", sent) - 1;
  len2 = snprintf(buffer2, sizeof(buffer2), "%d", recvd) - 1;
  tmp = jw_distance(buffer1, len1, buffer2, len2, opt);
  if (tmp > result) result = tmp;
  return result;
}

void
qso_exchange_probability(const struct Exchange_t * const sent,
			 const struct Exchange_t * const recvd,
			 const int                       isCW,
			 double                         *overallMetric,
			 double                         *callMetric)
{
  const char *sent_list[2];
  const char *recvd_list[2];
  int sent_len = pack_list(sent->d_basecall, sent->d_callsign, sent_list);
  int recvd_len = pack_list(recvd->d_basecall, recvd->d_callsign, recvd_list);
  *overallMetric = 0;
  *callMetric = max_match(sent_list, sent_len, recvd_list, recvd_len, isCW);
  if (*callMetric > 0) {
    *overallMetric = (*callMetric)*
      serialNumberCmp(sent->d_serial, recvd->d_serial, isCW);
    if (*overallMetric > 0) {
      sent_len = pack_list(sent->d_multiplier, sent->d_location, sent_list);
      recvd_len = pack_list(recvd->d_multiplier, recvd->d_location, recvd_list);
      *overallMetric = (*overallMetric) *
	max_match(sent_list, sent_len, recvd_list, recvd_len, isCW);
    }
  }
}

static double
timePenalty(time_t t1, time_t t2)
{
  const long diff = labs(t1 - t2);
  return (diff <= 15*60) ? 1.0 :
    ((diff >= (2*24*60*60)) ? 0.0 :
     (1.0 - (diff/(2.0*24*60*60))));
  
}

static double
qso_combined_probs(const double e1, const double e2,
		   const int bandmatch, const int modematch,
		   const double timepenalty)
{
  return e1*e2*(bandmatch ? 1.0 : 0.9)*(modematch ? 1.0 : 0.9)*
    timepenalty;
}

static VALUE
qso_probablymatch(VALUE obj, VALUE qso)
{
  const struct QSO_t *selfp, *qsop;
  double callOne, exchangeOne;
  double callTwo, exchangeTwo;
  VALUE result = rb_ary_new2(2);
  int isCW;
  GetQSO(obj, selfp);
  GetQSO(qso, qsop);
  isCW = ((m_CW == qsop->d_mode) && (m_CW == selfp->d_mode));
  if ((selfp == qsop) ||
      (selfp->d_logID == qsop->d_logID)) {
    rb_ary_store(result, 0, INT2FIX(0));
    rb_ary_store(result, 1, INT2FIX(0));
  }
  else {
    qso_exchange_probability(&(qsop->d_sent), &(selfp->d_recvd), isCW,
			     &exchangeOne, &callOne);
    qso_exchange_probability(&(qsop->d_recvd), &(selfp->d_sent), isCW,
			     &exchangeTwo, &callTwo);
    rb_ary_store(result, 1, 
		 rb_float_new(callOne*callTwo));
    rb_ary_store(result, 0,
		 rb_float_new(qso_combined_probs(exchangeOne, exchangeTwo,
						 selfp->d_band == qsop->d_band,
						 selfp->d_mode == qsop->d_mode,
						 timePenalty(selfp->d_datetime,
							     qsop->d_datetime))));
  }
  return result;
}

static double
serialMetric(const int16_t sent,
	     const int16_t recvd)
{
  if (sent >= 0) {
    if (recvd >= 0) {
      return abs(sent - recvd);
    }
    else {
      return abs(sent);
    }
  }
  return 0;
}

static double
strMetric(const int16_t sent,
	  const int16_t recvd,
	  const int     isCW)
{
  char buffer1[12], buffer2[12];
  int len1, len2;
  jw_Option opt = jw_option_new();
  opt.adj_table = isCW ? s_CW_adj: s_Normal_adj;
  if (sent >= 0) {
    len1 = snprintf(buffer1, sizeof(buffer1), "%" PRId16, sent) - 1;
  }
  else {
    len1 = 0;
    buffer1[0] = '\0';
  }
  if (recvd >= 0) {
    len2 = snprintf(buffer2, sizeof(buffer2), "%" PRId16, recvd) -1;
  }
  else{
    len2 = 0;
    buffer2[0] = '\0';
  }
  return jw_distance(buffer1, len1, buffer2, len2, opt);
}

static double
serialStrMetric(const int16_t s1,
		const int16_t r1,
		const int16_t s2,
		const int16_t r2,
		const int     isCW)
{
  return strMetric(s1,r2, isCW) * strMetric(s2, r1, isCW);
}

static double
qso_calc_ml_metrics(const struct QSO_t *selfp,
		    const struct QSO_t *qsop,
		    double metrics[11])
{
  const int isCW = ((m_CW == qsop->d_mode) && (m_CW == selfp->d_mode));
  double exchangeOne, callOne, exchangeTwo, callTwo, combined;
  qso_exchange_probability(&(qsop->d_sent), &(selfp->d_recvd), isCW,
			   &exchangeOne, &callOne);
  qso_exchange_probability(&(qsop->d_recvd), &(selfp->d_sent), isCW,
			   &exchangeTwo, &callTwo);
  combined = qso_combined_probs(exchangeOne, exchangeTwo,
				selfp->d_band == qsop->d_band,
				selfp->d_mode == qsop->d_mode,
				timePenalty(selfp->d_datetime,
					    qsop->d_datetime));
  if (combined > 0.1) {
    jw_Option opt = jw_option_new();
    opt.adj_table = isCW ? s_CW_adj : s_Normal_adj;
    metrics[0] = (selfp->d_band == qsop->d_band);
    metrics[1] = (selfp->d_mode == qsop->d_mode);
    metrics[2] = abs(selfp->d_datetime - qsop->d_datetime);
    metrics[3] = abs(selfp->d_frequency - qsop->d_frequency);
    metrics[4] = (serialMetric(selfp->d_sent.d_serial,
			       qsop->d_recvd.d_serial)+
		  serialMetric(qsop->d_sent.d_serial,
			       selfp->d_recvd.d_serial));
    metrics[5] = serialStrMetric(selfp->d_sent.d_serial,
				 selfp->d_recvd.d_serial,
				 qsop->d_sent.d_serial,
				 qsop->d_recvd.d_serial, isCW);
    metrics[6] = jw_distance(selfp->d_sent.d_multiplier,
			     strlen(selfp->d_sent.d_multiplier),
			     qsop->d_recvd.d_multiplier,
			     strlen(qsop->d_recvd.d_multiplier),
			     opt) *
      jw_distance(qsop->d_sent.d_multiplier,
		  strlen(qsop->d_sent.d_multiplier),
		  selfp->d_recvd.d_multiplier,
		  strlen(selfp->d_recvd.d_multiplier),
		  opt);
    metrics[7] = jw_distance(selfp->d_sent.d_location,
			     strlen(selfp->d_sent.d_location),
			     qsop->d_recvd.d_location,
			     strlen(qsop->d_recvd.d_location),
			     opt) *
      jw_distance(qsop->d_sent.d_location,
		  strlen(qsop->d_sent.d_location),
		  selfp->d_recvd.d_location,
		  strlen(selfp->d_recvd.d_location),
		  opt);
    metrics[8] = jw_distance(selfp->d_sent.d_basecall,
			     strlen(selfp->d_sent.d_basecall),
			     qsop->d_recvd.d_basecall,
			     strlen(qsop->d_recvd.d_basecall), opt) *
      jw_distance(qsop->d_sent.d_basecall,
		  strlen(qsop->d_sent.d_basecall),
		  selfp->d_recvd.d_basecall,
		  strlen(selfp->d_recvd.d_basecall), opt);
    metrics[9] = ((qsop->d_mode == m_CW) ? 1 : 0) +
      ((selfp->d_mode == m_CW) ? 1 : 0);
    metrics[10] = callOne*callTwo;
  }
  else {
    memset(metrics, 0, sizeof(double)*11);
  }
  return combined;
}

static VALUE
qso_ml_metrics(VALUE obj, VALUE qso)
{
  const struct QSO_t *selfp, *qsop;
  double combined, metrics[11];
  GetQSO(obj, selfp);
  GetQSO(qso, qsop);
  combined = qso_calc_ml_metrics(selfp, qsop, metrics);
  if (combined > 0.1) {
    VALUE result = rb_ary_new2(11);
    int i;
    for(i = 0; i < 11; ++i) {
      rb_ary_store(result, i, rb_float_new(metrics[i]));
    }
    return result;
  }
  return Qnil;
}

static VALUE
qso_ml_match_check(VALUE obj, VALUE qso)
{
  const struct QSO_t *selfp, *qsop;
  GetQSO(obj, selfp);
  GetQSO(qso, qsop);
  if ((selfp->d_logID != qsop->d_logID) && (selfp->d_qsoID < qsop->d_qsoID)) {
    double combined, metrics[11];
    combined = qso_calc_ml_metrics(selfp, qsop, metrics);
    if (combined > 0.1) {
      metrics[2] = 1.0e-3*metrics[2];
      metrics[3] = 1.0e-3*metrics[3];
      metrics[4] = 1.0e-3*metrics[4];
      if (qso_svm_match(s_svm_model, metrics)) return Qtrue;
    }
  }
  return Qnil;
}


static
enum Band_t
qso_lookupBand(VALUE band)
{
  if (T_STRING == TYPE(band)) {
    if ((RSTRING_LEN(band) >= 2) &&
	(RSTRING_LEN(band) < MAX_BAND_NAME)) {
      char bandbuf[MAX_BAND_NAME];
      int l=0, u=sizeof(s_bandMap)/sizeof(struct StringMap_t), m, cmp;
      memcpy(bandbuf, RSTRING_PTR(band), RSTRING_LEN(band));
      bandbuf[RSTRING_LEN(band)] = '\0';
      while (l < u) {		/* binary search */
	m = (l+u) >> 1;
	cmp = strcmp(bandbuf, s_bandMap[m].d_bandName);
	if (cmp > 0) l = m + 1;
	else if (cmp < 0) u = m;
	else
	  return s_bandMap[m].d_bandNum;
      }
    }
    rb_raise(rb_eQSOError, "Illegal QSO band '%s'", StringValueCStr(band) );
  }
  return b_Unknown;
}

static
enum Mode_t
qso_lookupMode(VALUE mode)
{
  if (T_STRING == TYPE(mode)) {
    if (2 == RSTRING_LEN(mode)) {
      char modebuf[3];
      int l=0, u=sizeof(s_modeMap)/sizeof(struct StringMap_t), m, cmp;
      memcpy(modebuf, RSTRING_PTR(mode), 2);
      modebuf[2] = '\0';
      while (l < u) {		/* binary search */
	m = (l+u) >> 1;
	cmp = strcmp(modebuf, s_modeMap[m].d_bandName);
	if (cmp > 0) l = m + 1;
	else if (cmp < 0) u = m;
	else
	  return s_modeMap[m].d_bandNum;
      }
    }
  }
  rb_raise(rb_eQSOError, "Illegal mode error");
  return m_RTTY;
}

static ID s_cToI;

static
time_t
qso_convertTime(VALUE datetime)
{
  VALUE time = rb_funcall(datetime, s_cToI, 0);
  return (time_t)NUM2LONG(time);
}

static void
qso_copystr(char *dest, VALUE str, const int maxchars, const char *field)
{
  if (T_STRING == TYPE(str)) {
    if (maxchars > RSTRING_LEN(str)) {
      memcpy(dest, RSTRING_PTR(str), RSTRING_LEN(str));
      dest[RSTRING_LEN(str)] = '\0';
    }
    else {
      memcpy(dest, RSTRING_PTR(str), maxchars - 1);
      dest[maxchars-1] = '\0';
    }
  }
  else {
    rb_raise(rb_eQSOError, "Incorrect argument for %s\n", field);
  }
}

static
void
fillOutExchange(struct Exchange_t *exch,
		VALUE basecall, VALUE call,
		VALUE serial, VALUE multiplier, VALUE location)
{
  if (NIL_P(basecall)) {
    exch->d_basecall[0] = '\0';	/* zero length string indicates NIL */
  }
  else {
    qso_copystr(exch->d_basecall, basecall, MAX_CALLSIGN_CHARS,"basecall");
  }
  if (NIL_P(call)) {
    exch->d_callsign[0] = '\0';	/* zero length string indicates NIL */
  }
  else {
    qso_copystr(exch->d_callsign, call, MAX_CALLSIGN_CHARS,"callsign");
  }
  if (NIL_P(serial)) {
    exch->d_serial = -1;	/* negative number indicates NIL */
  }
  else {
    exch->d_serial = (int32_t)NUM2LONG(serial);
  }
  if (NIL_P(multiplier)) {
    exch->d_multiplier[0] = '\0'; /* zero length string indicates NIL */
  }
  else {
    qso_copystr(exch->d_multiplier, multiplier, MAX_MULTIPLIER_CHARS, "multiplier");
  }
  if (NIL_P(location)) {
    exch->d_location[0] = '\0';
  }
  else {
    qso_copystr(exch->d_location, location, MAX_MULTIPLIER_CHARS, "location");
  }
}

/*
 * call-seq:
 *      QSO.new(id, logID, frequency, band, mode, datetime,
 *              sent_basecall, sent_call, sent_serial, sent_multiplier, sent_location,
 *              recvd_basecall, recvd_call, recvd_serial, recvd_multiplier, recvd_location)
 *
 * Initialize a new QSO object.
 */
static VALUE
qso_initialize(int argc, VALUE *argv, VALUE obj)
{
  if (16 == argc) {
    VALUE id = argv[0];
    VALUE logID = argv[1];
    VALUE frequency = argv[2];
    VALUE band = argv[3];
    VALUE mode = argv[4];
    VALUE datetime = argv[5];
    VALUE sent_basecall = argv[6];
    VALUE sent_call = argv[7];
    VALUE sent_serial = argv[8];
    VALUE sent_multiplier = argv[9];
    VALUE sent_location = argv[10];
    VALUE recvd_basecall = argv[11];
    VALUE recvd_call = argv[12];
    VALUE recvd_serial = argv[13];
    VALUE recvd_multiplier = argv[14];
    VALUE recvd_location = argv[15];
    struct QSO_t *qsop;
    GetQSO(obj, qsop);
    qsop->d_qsoID = (int32_t)NUM2LONG(id);
    qsop->d_logID = (int32_t)NUM2LONG(logID);
    qsop->d_frequency = (int32_t)NUM2LONG(frequency);
    qsop->d_band = qso_lookupBand(band);
    qsop->d_mode = qso_lookupMode(mode);
    qsop->d_datetime = qso_convertTime(datetime);
    fillOutExchange(&(qsop->d_sent),
		    sent_basecall, sent_call, sent_serial,
		    sent_multiplier, sent_location);
    fillOutExchange(&(qsop->d_recvd),
		    recvd_basecall, recvd_call, recvd_serial,
		    recvd_multiplier, recvd_location);
    return obj;
  }
  else {
    rb_raise(rb_eArgError, "Wrong number of arguments %d expected 16.",
	     argc);
  }
  return Qnil;
}

static VALUE
qso_cwjw(VALUE cls, VALUE s1, VALUE s2)
{
  if ((T_STRING == TYPE(s1)) && (T_STRING == TYPE(s2))) {
    const char *str1 = RSTRING_PTR(s1);
    const size_t str1len = RSTRING_LEN(s1);
    const char *str2 = RSTRING_PTR(s2);
    const size_t str2len = RSTRING_LEN(s2);
    jw_Option opt = jw_option_new();
    opt.adj_table = s_CW_adj;
    return rb_float_new(jw_distance(str1, str1len,
				    str2, str2len, opt));
  }
  rb_raise(rb_eTypeError, "Incorrect arguments to cwJaroWinkler (strings required).");
  return Qnil;
}

static VALUE
qso_phjw(VALUE cls, VALUE s1, VALUE s2)
{
  if ((T_STRING == TYPE(s1)) && (T_STRING == TYPE(s2))) {
    const char *str1 = RSTRING_PTR(s1);
    const size_t str1len = RSTRING_LEN(s1);
    const char *str2 = RSTRING_PTR(s2);
    const size_t str2len = RSTRING_LEN(s2);
    jw_Option opt = jw_option_new();
    opt.adj_table = s_Normal_adj;
    return rb_float_new(jw_distance(str1, str1len,
				    str2, str2len, opt));
  }
  rb_raise(rb_eTypeError, "Incorrect arguments to cwJaroWinkler (strings required).");
  return Qnil;
}
	       
void
Init_qsomatch(void)
{
  s_svm_model = qso_svm_classifier();
  s_CW_adj = jw_adj_matrix_create(jw_CW_ADJ_TABLE);
  s_Normal_adj = jw_adj_matrix_create(jw_HAM_ADJ_TABLE);
  rb_cQSO = rb_define_class("QSO", rb_cObject);
  rb_define_alloc_func(rb_cQSO, qso_s_allocate);
  rb_define_singleton_method(rb_cQSO, "toCW", qso_toCW, 1);
  rb_define_singleton_method(rb_cQSO, "cwJaroWinkler", qso_cwjw, 2);
  rb_define_singleton_method(rb_cQSO, "phJaroWinkler", qso_phjw, 2);
  rb_eQSOError = rb_define_class("QSOError", rb_eException);
  s_cToI = rb_intern("to_i");
  rb_define_method(rb_cQSO, "initialize", qso_initialize, -1);
  rb_define_method(rb_cQSO, "id", qso_id, 0);
  rb_define_method(rb_cQSO, "logID", qso_logID, 0);
  rb_define_method(rb_cQSO, "impossibleMatch?", qso_impossible, 1);
  rb_define_method(rb_cQSO, "ml_metrics", qso_ml_metrics, 1);
  rb_define_method(rb_cQSO, "freq", qso_freq, 0);
  rb_define_method(rb_cQSO, "band", qso_band, 0);
  rb_define_method(rb_cQSO, "mode", qso_mode, 0);
  rb_define_method(rb_cQSO, "datetime", qso_datetime, 0);
  rb_define_method(rb_cQSO, "recvd_basecall", qso_recvd_basecall, 0);
  rb_define_method(rb_cQSO, "recvd_callsign", qso_recvd_callsign, 0);
  rb_define_method(rb_cQSO, "recvd_serial", qso_recvd_serial, 0);
  rb_define_method(rb_cQSO, "recvd_multiplier", qso_recvd_multiplier, 0);
  rb_define_method(rb_cQSO, "recvd_location", qso_recvd_location, 0);
  rb_define_method(rb_cQSO, "sent_basecall", qso_sent_basecall, 0);
  rb_define_method(rb_cQSO, "sent_callsign", qso_sent_callsign, 0);
  rb_define_method(rb_cQSO, "sent_serial", qso_sent_serial, 0);
  rb_define_method(rb_cQSO, "sent_multiplier", qso_sent_multiplier, 0);
  rb_define_method(rb_cQSO, "sent_location", qso_sent_location, 0);
  rb_define_method(rb_cQSO, "basicLine", qso_basicLine, 0);
  rb_define_method(rb_cQSO, "to_s", qso_to_s, -1);
  rb_define_method(rb_cQSO, "fullMatch?", qso_fullmatch, 2);
  rb_define_method(rb_cQSO, "probablyMatch", qso_probablymatch, 1);
  rb_define_method(rb_cQSO, "svmMatch", qso_ml_match_check, 1);
}