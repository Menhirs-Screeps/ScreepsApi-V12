#include "Api-V12/Api.hpp"

#include "Api-V12/Routers/Client.hpp"

#include "ScreepsApi/Data.hpp"

namespace V12 {

nlohmann::json to_json (ScreepsApi::Reply reply) {
    return reply.data._json;
}

Api::Api(std::shared_ptr<ScreepsApi::Web::Client> pClient) :
    ScreepsApi::Api ( pClient ),
    m_client ( this ),
    m_initialized ( false )
{
    ScreepsApi::Data::Get().m_connected = false;
    ScreepsApi::Data::Get().m_workingDirectory = ".";
}

/*
*/

bool Api::Signin ( std::string email, std::string password )
{
    m_initialized = false;
    nlohmann::json content, reply;
    content["email"] = email;
    content["password"] = password;
    auto auth = reply = to_json ( m_client["api"]["auth"].route ( "signin", content.dump () ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1) return false;
    setToken (reply["token"].get<std::string>());
    auto user = reply = to_json ( m_client["api"]["auth"].route ( "me" ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1) return false;
    auto room = reply = to_json ( m_client["api"]["user"].route ( "world-start-room" ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1) return false;
    auto status = reply = to_json ( m_client["api"]["user"].route ( "world-status" ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1) return false;
    auto branches = reply = to_json ( m_client["api"]["user"].route ( "branches","{\"withCode\":false}" ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1) return false;
    ScreepsApi::Data::Get().setUser ( user );
    ScreepsApi::Data::Get().m_connected = true;
    m_initialized = true;
    return true;
}

bool Api::PullCode ( std::string branch )
{
    nlohmann::json content, reply;
    content["branch"] = branch;
    reply = to_json ( m_client["api"]["user"].route ( "code", content.dump (), ScreepsApi::Web::RoutingMethod::HttpGet ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1) return false;
    ScreepsApi::Data::Get().m_codeData = reply;
    ScreepsApi::Data::Get().m_code.FromJSON ( reply );
    return true;
}

bool Api::PushCode ( std::string branch )
{
    nlohmann::json content, reply;
    ScreepsApi::Data::Get().m_code.ToJSON ( content );
    reply = to_json ( m_client["api"]["user"].route ( "code", content.dump (), ScreepsApi::Web::RoutingMethod::HttpPost ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1) return false;
    return true;
}

bool Api::Console ( std::string command )
{
    nlohmann::json args;
    args["expression"] = command;
    nlohmann::json reply = to_json ( m_client["api"]["user"].route ( "console", args.dump () ) );
    return true;
}

bool Api::Room ( std::string name )
{
    nlohmann::json args;
    args["room"] = name;
    args["encoded"] = false;
    nlohmann::json reply = to_json ( m_client["api"]["game"].route ( "room-terrain", args.dump () ) );
    if ( reply.find ( "ok" ) == reply.end () ) return false;
    if ( reply["ok"].get<int>() != 1 ) return false;
    ScreepsApi::Data::Get ().setRoom ( reply["terrain"][0] );
    return true;
}

}