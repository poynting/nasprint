#!/usr/bin/env ruby
# -*- encoding: utf-8 -*-

require 'set'
require 'csv'
require_relative 'logset'

class Log
  def initialize(call, email, opclass)
    @call = call
    @email = email
    @opclass = opclass
    @numFull = 0
    @numBye = 0
    @numPartialBye = 0
    @numUnique = 0
    @numDupe = 0
    @numPartial = 0
    @numRemoved = 0
    @numNIL = 0
    @numOutsideContest = 0
    @multipliers = Set.new
  end

  # 
  def incCount(type)
    sym = ("@num" + type).to_s
    if instance_variable_defined?(sym)
      instance_variable_set(sym,1+instance_variable_get(sym))
    else
      print "Unknown QSO type #{type}\n"
    end
  end

  def addMultiplier(name)
    @multipliers.add(name)
  end

  def numqsos
    @numFull+@numBye+@numPartialBye/2-@numNIL
  end

  def nummultipliers
    @multipliers.size
  end

  def score
    numqsos * nummultipliers
  end

  def to_s
    "\"#{@call}\",#{@email ? ("\"" + @email + "\"") : ""},\"#{@opclass}\",#{@numFull},#{@numBye},#{@numPartial+@numPartialBye},#{@numUnique},#{@numDupe},#{@numPartial+@numRemoved},#{@numNIL},#{@numOutsideContest},#{@numFull+@numBye+@numPartialBye/2-@numNIL},#{@multipliers.size},#{(@numFull+@numBye+@numPartialBye/2-@numNIL)*@multipliers.size},\"#{@multipliers.to_a.sort.join(", ")}\""
  end
end

class Report
  def initialize(db, contestID)
    @db = db
    @contestID = contestID
  end

  def lookupMultiplier(id)
    @db.query("select m.abbrev, q.recvd_entityID from Multiplier as m, QSO as q where q.id = #{id} and  q.recvd_multiplierID = m.id limit 1;") { |row|
      return row[0], row[1]
    }
    return nil, nil
  end

  def addMultiplier(log, qsoID)
    abbrev, entity = lookupMultiplier(qsoID)
    if "DX" == abbrev
      if entity
        @db.query("select name, continent from Entity where id = ? limit 1;",
                        [entity]) { |row|
          if "NA" == row[1]     # it's a NA DX entity
            log.addMultiplier(row[0])
          end
        }
      else
        print "Log entry missing an entity number #{qsoID}.\n"
      end
    else
      if abbrev
        log.addMultiplier(abbrev)
      end
    end
  end

  MULTIPLIER_CREDIT = Set.new(%w( Full Partial Bye PartialBye)).freeze
  def scoreLog(id, log)
    @db.query("select q.matchType, q.id from QSO as q where q.logID = ? order by q.time asc;", [id]) { |row|
      log.incCount(row[0])
      if MULTIPLIER_CREDIT.include?(row[0]) # QSO counts for credit
        addMultiplier(log, row[1])
      end
    }
  end

  def toxicLogReport(out = $stdout, contestID)
    logs = Array.new
    @db.query("select l.callsign, l.callID, count(*) from Log as l, QSO as q where q.logID = l.id and contestID = ? group by l.id order by l.callsign asc;", [contestID]) { |row|
      item = Array.new(3)
      item[0] = row[0]
      item[1] = row[1].to_i
      item[2] = row[2].to_i
      logs << item
    }
    csv = CSV.new(out)
    csv << ["Callsign", "Claimed QSOs", "# in other logs", "# Full", "# Partial", "# NIL", "# Removed" ]
    logs.each { |l|
      @db.query("select count(*), sum(matchType = 'Full'), sum(matchType = 'Partial'), sum(matchType = 'NIL'), sum(matchType = 'Removed') from QSO where recvd_callID = ? group by recvd_callID limit 1;", [ l[1] ]) { |row|
        csv << [ l[0], l[2], row[0], row[1], row[2], row[3], row[4] ]
      }
    }
  end

  def makeReport(out = $stdout, contestID)
    logs = Array.new
    @db.query("select callsign, email, opclass, id from Log where contestID = ? order by callsign asc;", [contestID]) { |row|
      log = Log.new(row[0], row[1], row[2])
      scoreLog(row[3],log)
      @db.query("update Log set verifiedscore = #{log.score}, verifiedQSOs = ?, verifiedMultipliers = ? where id = ? limit 1;",
                [log.numqsos, log.nummultipliers, row[3]]) { }
      logs << log
    }
    out.write("\"Callsign\",\"Email\",\"Operator Class\",\"#Fully matched QSOs\",\"# Bye QSOs\",\"# Unique\",\"# Dupe\",\"# Incorrectly copied\",\"# NIL\",\"# Outside contest period\",\"WAS?\",\"# Verified QSOs (full+bye-NIL)\",\"# Verified Multipliers\",\"Verified Score\",\"Multipliers\"\r\n")
    logs.each { |log|
      out.write(log.to_s + "\r\n")
    }
  end
end
