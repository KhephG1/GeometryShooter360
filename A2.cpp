// Lecture7A2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//set origin function(x,y)
    //this function allows the location used as the position of the shape to be modified
    //using setorigin(radius,radius) we can use the center of the circle to track the position
#include <iostream>
#include <SFML/Graphics.hpp>
#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include<fstream>

class Entity;
class Vec2 {
public:
    float x{};
    float y{};
    Vec2() {}
    Vec2(float xin, float yin) : x{ xin }, y{ yin } {
    }
    bool operator == (const Vec2& rhs) const {

        return (x == rhs.x && y == rhs.y);
    }
    Vec2 operator+ (const Vec2& rhs) const {
        return Vec2{ x + rhs.x,y + rhs.y };
    }
    Vec2 operator-(const Vec2& rhs) const {
        return Vec2{ x - rhs.x,y - rhs.y };
    }
    Vec2 operator /(const float val) const {
        return Vec2{ x / val,y / val };
    }
    void operator += (const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
    }
    void operator -=(const Vec2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
    }
    void operator *=(const float val) {
        x *= val;
        y *= val;

    }
    void operator /=(const float val) {
        x /= val;
        y /= val;
    }
    Vec2 operator*(const float val) const {
        return Vec2{ x * val,y * val };
    }
    float dist(const Vec2& rhs) const {
        float dx = x - rhs.x;
        float dy = y - rhs.y;
        return sqrtf(dx * dx + dy * dy);
    }
};

std::ostream& operator<<(std::ostream& stream, Vec2 vec) {
    stream << "[" << vec.x << "," << vec.y << "]";
    return stream;
}
class CTransform {
public:
    Vec2 pos{ 0.0,0.0 };
    Vec2 velocity{ 0.0,0.0 };
    float angle{};

    CTransform(const Vec2& p, const Vec2& v, float a) : pos{ p }, velocity{ v }, angle{ a } {
    }
};
class CShape {
public:
    sf::CircleShape circle;
    CShape() {

    }
    CShape(float radius, size_t points, const sf::Color& fill, const sf::Color& outline, float thickness) : circle{ radius, points } {
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(thickness);
        circle.setOrigin(radius, radius);

    }
};
class CCollision {
public:
    float radius{};
    CCollision(float r) : radius{ r } {
    }
};

class CScore {
public:
    int score{};
    CScore(int s) :score{ s } {
    }
};
class CLifespan {
public:
    int remaining{};
    int total{};
    CLifespan(int total) : remaining{ total }, total{ total } {
    }
};
class CInput {
public:
    bool up{ false };
    bool down{ false };
    bool left{ false };
    bool right{ false };
    bool shoot{ false };
    CInput() {
    }
};
class Entity {
    friend class EntityManager;
    bool m_active{ true };
    size_t m_id{};
    std::string m_tag{ "default" };
    Entity(const size_t id, const std::string& tag) {

    }
public:
    bool is_special = false;
    std::shared_ptr<CTransform > cTransform;
    std::shared_ptr<CShape> cShape;
    std::shared_ptr<CCollision> cCollision;
    std::shared_ptr<CInput> cInput;
    std::shared_ptr<CScore> cScore;
    std::shared_ptr<CLifespan> cLifespan;
    bool isActive() const{
        return m_active;
    }
    const std::string& tag() const {
        return m_tag;
    }
    const size_t id() const {
        return m_id;
    }
    void destroy() {
        m_active = false;
    }
    void check_collision(sf::Window& w) {
        if (cTransform->pos.x - cCollision->radius <= 0||cTransform->pos.x+cCollision->radius >= w.getSize().x){
            cTransform->velocity.x *= -1;
        }
        if (cTransform->pos.y - cCollision->radius <= 0 || cTransform->pos.y + cCollision->radius >= w.getSize().y) {
            cTransform->velocity.y *= -1;
        }
    }
};

class EntityManager {
public:
    int m_totalEntities{};
    std::vector<std::shared_ptr<Entity>> m_entities{};
    std::vector<std::shared_ptr<Entity>> m_entitiesToAdd{};
    std::map< std::string, std::vector<std::shared_ptr<Entity>>> m_entityMap{};
    EntityManager() {
    }
    void update() {
        for (auto& e : m_entitiesToAdd) {
            m_entities.push_back(e);
        }
        removeDeadEntities(m_entities);
        for (auto& p : m_entityMap) {
            removeDeadEntities(p.second);
        }
    }
    void removeDeadEntities(std::vector<std::shared_ptr<Entity>>& vec) {
        std::vector < std::shared_ptr<Entity>> temp_vec{};
        for (auto& e : vec) {
            if (e->isActive() == true) {
                temp_vec.push_back(e);
            }
        }
        vec.clear();
        vec = temp_vec;
    }
    std::shared_ptr<Entity> addEntity(const std::string& tag) {
        auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
        m_entitiesToAdd.push_back(entity);
        m_entityMap[tag].push_back(entity);
        return entity;
    }

    const std::vector<std::shared_ptr<Entity>>& getEntities() {
        return m_entities;
    }
    const std::vector<std::shared_ptr<Entity>>& getEntities(const std::string& tag) {
        return m_entityMap[tag];
    }
};
struct PlayerConfig { int SR, CR, FR, FG,FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, Vmin, Vmax, L, SI; float Smin, Smax; };
struct BulletConfig { int SR, CR, FR, FG,FB, OR, OG, OB, OT, V, L; float S; };
class Game {
public:
    sf::RenderWindow m_window;
    EntityManager m_entities;
    sf::Font m_font;
    sf::Text m_text;
    PlayerConfig m_PlayerConfig;
    EnemyConfig  m_EnemyConfig;
    BulletConfig m_BulletConfig;
    int m_currentFrame{};
    int m_lastEnemySpawnTime{};
    bool m_paused{false};
    bool m_running{true};
    int m_score;
    int timer{ 400 };
    std::shared_ptr<Entity> m_player;

    Game(const std::string& config) {
        init(config);
    }
    void init(const std::string& path) {
        std::fstream f_in{ path };
        std::string temp{};
        while (f_in >> temp) {
            if (temp == "Window") {
                int width{}, height{}, frame_lim{}, mode{};
                f_in >> width>> height>> frame_lim>> mode;
                if (mode == 0) {
                    m_window.create(sf::VideoMode(width, height), "A2");
                }
                else {
                    m_window.create(sf::VideoMode(width, height), "A2", sf::Style::Fullscreen);
                }
                m_window.setFramerateLimit(frame_lim);
            }
            if (temp == "Player") {
                f_in >> m_PlayerConfig.SR >> m_PlayerConfig.CR >> m_PlayerConfig.S >> m_PlayerConfig.FR >> m_PlayerConfig.FG >> m_PlayerConfig.FB >> m_PlayerConfig.OR >> m_PlayerConfig.OG >> m_PlayerConfig.OB >> m_PlayerConfig.OT >> m_PlayerConfig.V;
            }
            if (temp == "Bullet") {
                f_in >> m_BulletConfig.SR >> m_BulletConfig.CR >> m_BulletConfig.S >> m_BulletConfig.FR >> m_BulletConfig.FG >> m_BulletConfig.FB >> m_BulletConfig.OR >> m_BulletConfig.OG >> m_BulletConfig.OB >> m_BulletConfig.OT >> m_BulletConfig.V >> m_BulletConfig.L;
            }
            if (temp == "Enemy") {
                f_in >> m_EnemyConfig.SR >> m_EnemyConfig.CR >> m_EnemyConfig.Smin >> m_EnemyConfig.Smax >> m_EnemyConfig.OR >> m_EnemyConfig.OG >> m_EnemyConfig.OB >> m_EnemyConfig.OT >> m_EnemyConfig.Vmin >> m_EnemyConfig.Vmax >> m_EnemyConfig.L >> m_EnemyConfig.SI;
            }
        }
        spawnPlayer();
        
    }
    void togglePaused() {
        if (m_paused == false) {
            m_paused = true;
        }
        else {
            m_paused = false;
        }
    }
    void sRender() {
        m_window.clear();
        sf::Font F{};
        F.loadFromFile("c:/fonts/NotoSans-Regular.ttf");
        sf::Text tex{"special ability timer: "  + std::to_string(timer),F,24};
        tex.setPosition(m_window.getSize().x - 300, 0);
        m_window.draw(tex);
        if (timer > 0) {
            timer--;
        }
        for (auto& e : m_entities.getEntities()) {
            e ->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
            e->cTransform->angle += 5;
            e->cShape->circle.setRotation(e->cTransform->angle);
            m_window.draw(e->cShape->circle);
        }
        if (m_paused) {
            sf::Font f{};
            f.loadFromFile("c:/fonts/NotoSans-Regular.ttf");
            sf::Text text{"Game Paused", f, 50};
            text.setPosition((m_window.getSize().x / 2.0)-(text.getGlobalBounds().getSize().x)/2.0, (m_window.getSize().y / 2.0)- (text.getGlobalBounds().getSize().y)/2.0);
            m_window.draw(text);
        }
        
        spawnscore();
        m_window.display();
    }
    void sCollision() {
        
        for (auto& b : m_entities.getEntities("bullet")) {
            for (auto& e : m_entities.getEntities("enemy")) {
                //add collision logic
                if (b->cTransform->pos.dist(e->cTransform->pos) < b->cCollision->radius + e->cCollision->radius) {
                    m_player->cScore->score += e->cShape->circle.getPointCount() * 100;
                    spawnSmallEnemies(e);
                    b->destroy();
                    e->destroy();
                }    
            }
        }
        for (auto& e : m_entities.getEntities("enemy")) {
            if (e->cTransform->pos.dist(m_player->cTransform->pos) < e->cCollision->radius + m_player->cCollision->radius) {
                m_running = false;
                }
            
        }
        for (auto& e : m_entities.getEntities("smallEnemy")) {
            if (e->cTransform->pos.dist(m_player->cTransform->pos) < e->cCollision->radius + m_player->cCollision->radius) {
                m_running = false;
            }
        }
    }
    void sMovement() {
        
        float upper_b{ m_player->cTransform->pos.y - m_player->cCollision->radius };
        float lower_b{ m_player->cTransform->pos.y + m_player->cCollision->radius };
        float righ_b{ m_player->cTransform->pos.x + m_player->cCollision->radius };
        float left_b{ m_player->cTransform->pos.x - m_player->cCollision->radius };
        if (m_player->cInput->up && upper_b>=0) {
            m_player->cTransform->velocity.y = -10;
        }
        else if (m_player->cInput->down && lower_b <= m_window.getSize().y) {
            m_player->cTransform->velocity.y = 10;
        }
        else {
            m_player->cTransform->velocity.y = 0;
        }
        if (m_player->cInput->left && left_b>=0) {
            m_player->cTransform->velocity.x = -10;
        }
        else if (m_player->cInput->right && righ_b <= m_window.getSize().x) {
            m_player->cTransform->velocity.x = 10;
        }
        else {
            m_player->cTransform->velocity.x = 0;
        }
        for (auto& ent : m_entities.getEntities("bullet")) {
            ent->cTransform->pos.x += ent->cTransform->velocity.x;
            ent->cTransform->pos.y += ent->cTransform->velocity.y;
            if (ent->is_special) {
                ent->cCollision->radius += 1;
                ent->cShape->circle.setRadius(ent->cShape->circle.getRadius() + 0.2);
            }
            ent->check_collision(m_window);
        }
        for (auto& ent : m_entities.getEntities("enemy")) {
            ent->cTransform->pos.x += ent->cTransform->velocity.x;
            ent->cTransform->pos.y += ent->cTransform->velocity.y;
            ent->check_collision(m_window);
        }
        for (auto& ent : m_entities.getEntities("smallEnemy")) {
            ent->cTransform->pos.x += ent->cTransform->velocity.x;
            ent->cTransform->pos.y += ent->cTransform->velocity.y;
            ent->check_collision(m_window);
        }
        m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
        m_player->cTransform->pos.y += m_player->cTransform->velocity.y;
        

    }
    void sLifespan() {
        for (auto& ent : m_entities.getEntities("bullet")) {
            ent->cLifespan->remaining -= 1;
            auto r = ent->cShape->circle.getFillColor().r;
            auto g = ent->cShape->circle.getFillColor().g;
            auto b = ent->cShape->circle.getFillColor().b;
            sf::Color c{ r, g, b,  (sf::Uint8)(255 * (ent->cLifespan->remaining) / (float)ent->cLifespan->total) };
            if (!ent->is_special) {
                ent->cShape->circle.setOutlineColor(c);
                ent->cShape->circle.setFillColor(c);
            }
            if (ent->cLifespan->remaining <= 0) {
                ent->destroy();
            }
        }
        for (auto& ent : m_entities.getEntities("smallEnemy")) {
            ent->cLifespan->remaining -=1;
            auto r = ent->cShape->circle.getFillColor().r; 
            auto g = ent->cShape->circle.getFillColor().g;   
            auto b = ent->cShape->circle.getFillColor().b;
            sf::Color c{ r, g, b, (sf::Uint8)(255 * (ent->cLifespan->remaining) / (float)ent->cLifespan->total)};
            ent->cShape->circle.setOutlineColor(c);
            ent->cShape->circle.setFillColor(c);
            if (ent->cLifespan->remaining <= 0) {
                ent->destroy();
            }
        }

    }
    void sEnemySpawner() {
        if (m_currentFrame - m_lastEnemySpawnTime > 100) {
            spawnEnemy();
        }
    }
    void spawnPlayer() {
        
        auto entity{ m_entities.addEntity("player") };
        float mx = m_window.getSize().x / 2.0;
        float my = m_window.getSize().y / 2.0;
        entity->cTransform = std::make_shared<CTransform>(Vec2{ mx,my }, Vec2{ 50,50}, 3.0f);
        entity->cScore = std::make_shared<CScore>(0);
        entity->cShape = std::make_shared<CShape>(m_PlayerConfig.SR, m_PlayerConfig.V, sf::Color{ (sf::Uint8)m_PlayerConfig.FR, (sf::Uint8)m_PlayerConfig.FG, (sf::Uint8)m_PlayerConfig.FB }, sf::Color{ (sf::Uint8)m_PlayerConfig.OR,(sf::Uint8)m_PlayerConfig.OG,(sf::Uint8)m_PlayerConfig.OB }, m_PlayerConfig.OT);
        entity->cInput = std::make_shared<CInput>();
        entity->cCollision = std::make_shared<CCollision>(m_PlayerConfig.CR);
        m_player = entity;
        
    }
    void spawnEnemy() {
        
        auto entity = m_entities.addEntity("enemy");
        entity->cShape = std::make_shared<CShape>(m_EnemyConfig.SR, (rand() % (m_EnemyConfig.Vmax - m_EnemyConfig.Vmin))+m_EnemyConfig.Vmin, sf::Color((sf::Uint8)((rand() % 255 - 10) + 10), (sf::Uint8)((rand() % 255 - 10) + 10), (sf::Uint8)((rand() % 255 - 10) + 10)), sf::Color(255, 255, 255), 1.0f);
        entity->cCollision = std::make_shared<CCollision>(m_EnemyConfig.CR);
        float ex = (rand() % (int)(m_window.getSize().x + 1 - entity->cCollision->radius)) + entity->cCollision->radius;
        float ey = (rand() % (int)(m_window.getSize().y + 1 - entity->cCollision->radius)) + entity->cCollision->radius;
        if (ex >= m_player->cTransform->pos.x - m_player->cShape->circle.getRadius() && ex <= m_player->cTransform->pos.x + m_player->cShape->circle.getRadius()) {
            ex += (m_player->cShape->circle.getRadius() * 2);
        }
        if (ey >= m_player->cTransform->pos.y - m_player->cShape->circle.getRadius() && ey <= m_player->cTransform->pos.y + m_player->cShape->circle.getRadius()) {
            ey += (m_player->cShape->circle.getRadius() * 7);
        }
       
        float speed = (rand() % (int)(m_EnemyConfig.Smax - m_EnemyConfig.Smin)) + m_EnemyConfig.Smin;
        float angle = rand() % 8 * atan(1);
        entity->cTransform = std::make_shared<CTransform>(Vec2{ ex,ey }, Vec2{ speed * cos(angle), speed * sin(angle) }, angle);
        m_lastEnemySpawnTime = m_currentFrame;
        
    }
    void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target) {
        float speed = m_BulletConfig.S;
        auto bullet = m_entities.addEntity("bullet");
        Vec2 path = target - entity->cTransform->pos;
        float angle = acos(std::abs(path.x) / entity->cTransform->pos.dist(target));
        if (path.x < 0 && path.y>0) {
            angle = 4*atan(1) - angle;
        }
        if (path.x < 0 && path.y < 0) {
            angle += 4*atan(1);
        }
        if (path.x > 0 && path.y < 0){
            angle = 8*atan(1) - angle; 
        }
        bullet->cTransform = std::make_shared<CTransform>(Vec2{ entity->cTransform->pos.x,entity->cTransform->pos.y }, Vec2{ speed * cos(angle), speed * sin(angle) }, angle);
        bullet->cShape = std::make_shared<CShape>(m_BulletConfig.SR, m_BulletConfig.V, sf::Color((sf::Uint8)m_BulletConfig.FR, (sf::Uint8)m_BulletConfig.FG, (sf::Uint8)m_BulletConfig.FG ), sf::Color((sf::Uint8)m_BulletConfig.OR, (sf::Uint8)m_BulletConfig.OG, (sf::Uint8)m_BulletConfig.OB), (sf::Uint8)m_BulletConfig.OT);
        bullet->cCollision = std::make_shared<CCollision>(m_BulletConfig.CR);
        bullet->cLifespan = std::make_shared<CLifespan>(m_BulletConfig.L);
    }


    void spawnSmallEnemies(std::shared_ptr<Entity> e) {
        for (int i{}; i < e->cShape->circle.getPointCount(); i++) {
            float speed = 5.0f;
            float angle = ((8*atan(1))/e->cShape->circle.getPointCount()) * i;
            auto entity{ m_entities.addEntity("smallEnemy") };
            entity->cTransform = std::make_shared<CTransform>(Vec2{ e->cTransform->pos.x,e->cTransform->pos.y }, Vec2{ speed * cos(angle), speed * sin(angle) }, angle);
            entity->cShape = std::make_shared<CShape>(e->cShape->circle.getRadius() / 2.0f, e->cShape->circle.getPointCount(), sf::Color(e->cShape->circle.getFillColor().r, e->cShape->circle.getFillColor().g, e->cShape->circle.getFillColor().b), sf::Color(e->cShape->circle.getOutlineColor().r, e->cShape->circle.getOutlineColor().g, e->cShape->circle.getOutlineColor().b), m_EnemyConfig.OT);
            entity->cCollision = std::make_shared<CCollision>(e->cShape->circle.getRadius()/2.0);
            entity->cLifespan = std::make_shared<CLifespan>(m_EnemyConfig.L);
        }
    }
    void spawnSpeicalWeapon(std::shared_ptr<Entity> e) {
        for (int i{}; i < e->cShape->circle.getPointCount(); i++) {
            float speed = 15.0;
            float angle = ((8 * atan(1)) / e->cShape->circle.getPointCount()) * i;
            auto entity{ m_entities.addEntity("bullet") };
            entity->cTransform = std::make_shared<CTransform>(Vec2{ e->cTransform->pos.x,e->cTransform->pos.y }, Vec2{ speed * cos(angle), speed * sin(angle) }, angle);
            entity->cShape = std::make_shared<CShape>(m_BulletConfig.SR, 3, sf::Color(255,0,0), sf::Color(255,0,0), m_EnemyConfig.OT);
            entity->cCollision = std::make_shared<CCollision>(m_BulletConfig.SR);
            entity->cLifespan = std::make_shared<CLifespan>(m_BulletConfig.L*0.5);
            entity->is_special = true;
        }

    }
    void spawnscore() {
        
        sf::Font myFont{};
        myFont.loadFromFile("c:/fonts/NotoSans-Regular.ttf");
        sf::Text player_score{"Player Score: "+std::to_string(m_player->cScore->score), myFont, 24};
        m_window.draw(player_score);
        
    }
    void sUserInput() {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed||event.key.code==sf::Keyboard::Escape) {
                m_running = false;
            }
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::W:
                    m_player->cInput->up = true;
                    break;
                case sf::Keyboard::A:
                    m_player->cInput->left = true;
                    break;
                case sf::Keyboard::S:
                    m_player->cInput->down = true;
                    break;
                case sf::Keyboard::D:
                    m_player->cInput->right = true;
                    break;
                case sf::Keyboard::P:
                    togglePaused();
                case sf::Keyboard::Q:
                    if (timer == 0) {
                        spawnSpeicalWeapon(m_player);
                        timer = 400;
                    }
                    break;
                default: break;
                }
            }
            if (event.type == sf::Event::KeyReleased) {
                switch (event.key.code) {
                case sf::Keyboard::W:
                    m_player->cInput->up = false;
                    break;
                case sf::Keyboard::A:
                    m_player->cInput->left = false;
                    break;
                case sf::Keyboard::S:
                    m_player->cInput->down = false;
                    break;
                case sf::Keyboard::D:
                    m_player->cInput->right = false;
                    break;
                default: break;
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                switch (event.mouseButton.button) {
                case sf::Mouse::Left:
                    m_player->cInput->shoot = true;
                    spawnBullet(m_player,Vec2{ (float)event.mouseButton.x,(float)event.mouseButton.y });
                    break;
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                switch (event.mouseButton.button) {
                case sf::Mouse::Left:
                    m_player->cInput->shoot = false;
                    break;
                }
            }


        }
    }
    void run() {
        while (m_running) {
            if (!m_paused) {
                m_entities.update();
                m_entities.m_entitiesToAdd.clear();
                sMovement();
                sCollision();
                sLifespan();
                sEnemySpawner();
                m_currentFrame++;
            }
            sRender();
            sUserInput();
          
        }
    }
};
int main(){
    
    Game g("config.txt");
    g.run();
    int sc{ g.m_player->cScore->score };
    if (sc < 20000) {
        std::cout << "You Suck (Score: " + std::to_string(sc) << ")" << std::endl;
    }
    else {
        std::cout<< "You Don't Suck (Score: " + std::to_string(sc) << ")" << std::endl;
    }
    
    return 0;
}

