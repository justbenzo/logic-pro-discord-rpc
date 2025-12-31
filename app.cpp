#define DISCORDPP_IMPLEMENTATION
#include "discordpp.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include <csignal>
#include <chrono>
#include <memory>
#include <mutex>

#include "logiccheck.h"
#include "projectname.h"
#include "token_store.h"

std::thread logic_thread;
static const uint64_t APPLICATION_ID = 1454592403628359872;
std::atomic<bool> running = true;

static std::shared_ptr<discordpp::Client> g_client;
static std::mutex g_client_mtx;

void signalHandler(int) { running.store(false); }

void logic_pro_loop(std::shared_ptr<discordpp::Client> client) {
    bool was_running = false;

    while (running.load()) {
        bool is_running = is_logic_pro_running();

        if (!is_running) {
            if (was_running) {
                client->ClearRichPresence(); // ✅ actually clears it
            }
            was_running = false;

            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        was_running = true;

        std::string project = get_logic_project_name();

        discordpp::Activity activity;
        activity.SetType(discordpp::ActivityTypes::Playing);

        if (!project.empty()) {
            activity.SetState("working on " + project);
        } else {
            activity.SetState("browsing projects");
        }

        client->UpdateRichPresence(activity, [](discordpp::ClientResult r) {
            if (!r.Successful()) std::cerr << "❌ set presence failed\n";
        });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}




void app_init() {
    std::signal(SIGINT, signalHandler);

    auto client = std::make_shared<discordpp::Client>();
    {
        std::lock_guard<std::mutex> lk(g_client_mtx);
        g_client = client;
    }

    client->AddLogCallback([](auto message, auto severity) {
        std::cout << "[" << EnumToString(severity) << "] " << message << std::endl;
    }, discordpp::LoggingSeverity::Info);

    client->SetStatusChangedCallback([client](discordpp::Client::Status status,
                                              discordpp::Client::Error error,
                                              int32_t errorDetail) {
        std::cout << "🔄 Status changed: " << discordpp::Client::StatusToString(status) << std::endl;

        if (status == discordpp::Client::Status::Ready) {
            std::cout << "✅ Client is ready! Starting Logic Pro monitor...\n";
            if (!logic_thread.joinable()) {
                logic_thread = std::thread(logic_pro_loop, client);
            }
        } else if (error != discordpp::Client::Error::None) {
            std::cerr << "❌ Connection Error: "
                      << discordpp::Client::ErrorToString(error)
                      << " - Details: " << errorDetail << std::endl;
        }
    });
}

void app_try_silent_auth() {
    std::shared_ptr<discordpp::Client> client;
    {
        std::lock_guard<std::mutex> lk(g_client_mtx);
        client = g_client;
    }
    if (!client) return;

    std::string saved_refresh = token_store::load_refresh_token();
    std::cerr << "Saved refresh token len=" << saved_refresh.size() << "\n";

    if (saved_refresh.empty()) return;

    std::cout << "🔁 Silent auth: RefreshToken...\n";

    client->RefreshToken(APPLICATION_ID, saved_refresh,
        [client](discordpp::ClientResult result,
                 std::string accessToken,
                 std::string newRefreshToken,
                 discordpp::AuthorizationTokenType,
                 int32_t,
                 std::string) {

            if (!result.Successful()) {
                std::cerr << "❌ Silent RefreshToken failed (will require Authenticate…)\n";
                return;
            }

            if (!newRefreshToken.empty()) token_store::save_refresh_token(newRefreshToken);

            client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                [client](discordpp::ClientResult r) {
                    if (r.Successful()) {
                        std::cout << "✅ Silent token updated, connecting...\n";
                        client->Connect();
                    } else {
                        std::cerr << "❌ UpdateToken failed after silent refresh\n";
                    }
                }
            );
        }
    );
}

void app_begin_auth() {
    std::shared_ptr<discordpp::Client> client;
    {
        std::lock_guard<std::mutex> lk(g_client_mtx);
        client = g_client;
    }
    if (!client) return;

    std::string saved_refresh = token_store::load_refresh_token();
    if (!saved_refresh.empty()) {
        std::cout << "🔁 Trying RefreshToken (silent)...\n";
        client->RefreshToken(APPLICATION_ID, saved_refresh,
            [client](discordpp::ClientResult result,
                     std::string accessToken,
                     std::string newRefreshToken,
                     discordpp::AuthorizationTokenType,
                     int32_t,
                     std::string) {

                if (!result.Successful()) {
                    std::cerr << "❌ RefreshToken failed, need browser auth\n";
                    return;
                }

                if (!newRefreshToken.empty()) token_store::save_refresh_token(newRefreshToken);

                client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                    [client](discordpp::ClientResult r) {
                        if (r.Successful()) {
                            std::cout << "✅ Token updated, connecting...\n";
                            client->Connect();
                        } else {
                            std::cerr << "❌ UpdateToken failed after refresh\n";
                        }
                    }
                );
            }
        );
        return;
    }

    std::cout << "🌐 No saved refresh token, starting browser auth...\n";
    auto codeVerifier = client->CreateAuthorizationCodeVerifier();

    discordpp::AuthorizationArgs args{};
    args.SetClientId(APPLICATION_ID);
    args.SetScopes({ "openid", "sdk.social_layer_presence" });
    args.SetCodeChallenge(codeVerifier.Challenge());

    client->Authorize(args, [client, codeVerifier](auto result, auto code, auto redirectUri) {
        if (!result.Successful()) {
            std::cerr << "❌ Authentication Error: " << result.Error() << std::endl;
            return;
        }

        client->GetToken(
            APPLICATION_ID, code, codeVerifier.Verifier(), redirectUri,
            [client](discordpp::ClientResult result,
                     std::string accessToken,
                     std::string refreshToken,
                     discordpp::AuthorizationTokenType,
                     int32_t,
                     std::string) {

                if (!result.Successful()) {
                    std::cerr << "❌ GetToken failed\n";
                    return;
                }

                if (!refreshToken.empty()) token_store::save_refresh_token(refreshToken);

                client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                    [client](discordpp::ClientResult r) {
                        if (r.Successful()) {
                            std::cout << "✅ Token updated, connecting...\n";
                            client->Connect();
                        } else {
                            std::cerr << "❌ UpdateToken failed\n";
                        }
                    }
                );
            }
        );
    });
}

int app_main() {
    std::cout << "🚀 Initializing Discord SDK...\n";
    app_init();

    app_try_silent_auth();

    while (running.load()) {
        discordpp::RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (logic_thread.joinable()) logic_thread.join();
    return 0;
}
